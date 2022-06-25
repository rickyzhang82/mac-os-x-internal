// exception.c

#include "exc.h"
#include "mach_exc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mach/mach.h>
#include <capstone.h>
   
// exception message we will receive from the kernel
typedef struct exc_msg {
    mach_msg_header_t          Head;
    mach_msg_body_t            msgh_body; // start of kernel-processed data
    mach_msg_port_descriptor_t thread;    // victim thread
    mach_msg_port_descriptor_t task;      // end of kernel-processed data
    NDR_record_t               NDR;       // see osfmk/mach/ndr.h
    exception_type_t           exception;
    mach_msg_type_number_t     codeCnt;   // number of elements in code[]
    exception_data_t           code;      // an array of integer_t
    char                       pad[512];  // for avoiding MACH_MSG_RCV_TOO_LARGE
} exc_msg_t;
   
// reply message we will send to the kernel
typedef struct rep_msg {
    mach_msg_header_t          Head;
    NDR_record_t               NDR;       // see osfmk/mach/ndr.h
    kern_return_t              RetCode;   // indicates to the kernel what to do
} reply_msg_t;
   
// exception handling
mach_port_t exception_port;
void exception_handler(void);


extern boolean_t exc_server(mach_msg_header_t *request,
                            mach_msg_header_t *reply);

// Routine mach_exception_raise
extern kern_return_t
catch_mach_exception_raise(mach_port_t exception_port, mach_port_t thread,
                           mach_port_t task, exception_type_t exception,
                           mach_exception_data_t code,
                           mach_msg_type_number_t codeCnt);

extern kern_return_t
catch_mach_exception_raise_state(
    mach_port_t exception_port, exception_type_t exception,
    const mach_exception_data_t code, mach_msg_type_number_t codeCnt,
    int *flavor, const thread_state_t old_state,
    mach_msg_type_number_t old_stateCnt, thread_state_t new_state,
    mach_msg_type_number_t *new_stateCnt);

// Routine mach_exception_raise_state_identity
extern kern_return_t
catch_mach_exception_raise_state_identity(
    mach_port_t exception_port, mach_port_t thread, mach_port_t task,
    exception_type_t exception, mach_exception_data_t code,
    mach_msg_type_number_t codeCnt, int *flavor, thread_state_t old_state,
    mach_msg_type_number_t old_stateCnt, thread_state_t new_state,
    mach_msg_type_number_t *new_stateCnt);

extern boolean_t mach_exc_server(   mach_msg_header_t *InHeadP,
                                     mach_msg_header_t *OutHeadP);

// demonstration function and associates
typedef void  (* funcptr_t)(void);
funcptr_t     function_with_bad_instruction;
kern_return_t repair_instruction(mach_port_t victim);
void          graceful_dead(void);

#if (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))
int instruction_length_at_address(uint64_t address);
#endif

// support macros for pretty printing
#define L_MARGIN "%-21s: "
#define FuncPutsN(msg)   printf(L_MARGIN "%s", __FUNCTION__, msg)
#define FuncPuts(msg)    printf(L_MARGIN "%s\n", __FUNCTION__, msg)
#define FuncPutsIDs(msg) printf(L_MARGIN "%s (task %#lx, thread %#lx)\n", \
                                __FUNCTION__, msg, (long)mach_task_self(), \
                                (long)pthread_mach_thread_np(pthread_self()));
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
int
main(int argc, char **argv)
{
    kern_return_t kr; 
    pthread_t     exception_thread;
    mach_port_t   mytask = mach_task_self();
    mach_port_t   mythread = mach_thread_self();
   
    FuncPutsIDs("starting up");
   
    // create a receive right
    kr = mach_port_allocate(mytask, MACH_PORT_RIGHT_RECEIVE, &exception_port);
    EXIT_ON_MACH_ERROR("mach_port_allocate", kr);
   
    // insert a send right: we will now have combined receive/send rights
    kr = mach_port_insert_right(mytask, exception_port, exception_port,
                                MACH_MSG_TYPE_MAKE_SEND);
    OUT_ON_MACH_ERROR("mach_port_insert_right", kr);
   
#if (defined(powerpc) || defined(__powerpc__))

    kr = thread_set_exception_ports(mythread,                   // target thread
                                    EXC_MASK_BAD_INSTRUCTION,   // exception types in PPC
                                    exception_port,             // the port
                                    EXCEPTION_DEFAULT,          // behavior
                                    THREAD_STATE_NONE);         // flavor

#elif (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))

    kr = thread_set_exception_ports(mythread,                                       // target thread
                                    EXC_MASK_BAD_ACCESS,                            // exception types in Apple Silicon and x86 64
                                    exception_port,                                 // the port
                                    EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES,       // behavior, for 64bit machine it must use MACH_EXCEPTION_CODES
                                                                                    // if it use MIG to generate mach_exc_server
                                    THREAD_STATE_NONE);                             // flavor

#endif

    OUT_ON_MACH_ERROR("thread_set_exception_ports", kr);
   
    if ((pthread_create(&exception_thread, (pthread_attr_t *)0,
                        (void *(*)(void *))exception_handler, (void *)0))) {
        perror("pthread_create");
        goto out;
    }
   
    FuncPuts("about to dispatch exception_handler pthread");
    pthread_detach(exception_thread);
   
    // some random bad address for code, but otherwise a valid address
    function_with_bad_instruction = (funcptr_t)exception_thread;
   
    FuncPuts("about to call function_with_bad_instruction");
    function_with_bad_instruction();
    FuncPuts("after function_with_bad_instruction");
   
out:
    mach_port_deallocate(mytask, mythread);
    if (exception_port)
        mach_port_deallocate(mytask, exception_port);
   
    return 0;
}
   
void
exception_handler(void)
{
    kern_return_t kr;
    exc_msg_t     msg_recv;
    reply_msg_t   msg_resp;
   
    FuncPutsIDs("beginning");
    
    msg_recv.Head.msgh_local_port = exception_port;
    msg_recv.Head.msgh_size = sizeof(msg_recv);

#if (defined(powerpc) || defined(__powerpc__))
    kr = mach_msg(&(msg_recv.Head),            // message
                  MACH_RCV_MSG|MACH_RCV_LARGE, // options
                  0,                           // send size (irrelevant here)
                  sizeof(msg_recv),            // receive limit
                  exception_port,              // port for receiving
                  MACH_MSG_TIMEOUT_NONE,       // no timeout
                  MACH_PORT_NULL);             // notify port (irrelevant here)

#elif (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))
    kr = mach_msg_receive(&(msg_recv.Head));
#endif
    EXIT_ON_MACH_ERROR("mach_msg_receive", kr);
   
    FuncPuts("received message");
    FuncPutsN("victim thread is ");
    printf("%#lx\n", (long)msg_recv.thread.name);
    FuncPutsN("victim thread's task is ");
    printf("%#lx\n", (long)msg_recv.task.name);

#if (defined(powerpc) || defined(__powerpc__))
    FuncPutsIDs("calling exc_server");
    exc_server(&msg_recv.Head, &msg_resp.Head);
    // now msg_resp.RetCode contains return value of catch_exception_raise()
#elif (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))
    FuncPutsIDs("calling mach_exc_server");
    mach_exc_server(&msg_recv.Head, &msg_resp.Head);
#endif
    FuncPuts("sending reply");
    kr = mach_msg(&(msg_resp.Head),        // message
                  MACH_SEND_MSG,           // options
                  msg_resp.Head.msgh_size, // send size
                  0,                       // receive limit (irrelevant here)
                  MACH_PORT_NULL,          // port for receiving (none)
                  MACH_MSG_TIMEOUT_NONE,   // no timeout
                  MACH_PORT_NULL);         // notify port (we don't want one)
    EXIT_ON_MACH_ERROR("mach_msg_send", kr);
    FuncPutsIDs("sent reply and exiting the thread");
    pthread_exit((void *)0);
}


#if (defined(powerpc) || defined(__powerpc__))

kern_return_t
catch_exception_raise(mach_port_t            port,
                      mach_port_t            victim,
                      mach_port_t            task,
                      exception_type_t       exception,
                      exception_data_t       code,
                      mach_msg_type_number_t code_count)
{
    FuncPutsIDs("beginning");
    if (exception != EXC_BAD_INSTRUCTION) {
        // this should not happen, but we should forward an exception that we
        // were not expecting... here, we simply bail out
        FuncPuts("caught an unexpected exception!");
        exit(-1);
    }
   
    return repair_instruction(victim);
}

#elif (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))

kern_return_t
catch_mach_exception_raise(mach_port_t              port,
                           mach_port_t              victim,
                           mach_port_t              task,
                           exception_type_t         exception,
                           mach_exception_data_t    code,
                           mach_msg_type_number_t   code_count)
{
    FuncPutsIDs("beginning");
    if (exception != EXC_BAD_ACCESS) {
        // this should not happen, but we should forward an exception that we
        // were not expecting... here, we simply bail out
        FuncPuts("caught an unexpected exception!");
        exit(-1);
    }

    return repair_instruction(victim);
}

kern_return_t catch_mach_exception_raise_state(
    mach_port_t exception_port, exception_type_t exception,
    const mach_exception_data_t code, mach_msg_type_number_t codeCnt,
    int *flavor, const thread_state_t old_state,
    mach_msg_type_number_t old_stateCnt, thread_state_t new_state,
    mach_msg_type_number_t *new_stateCnt)
{
    FuncPutsIDs("beginning");
    return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_identity(
    mach_port_t exception_port, mach_port_t thread, mach_port_t task,
    exception_type_t exception, mach_exception_data_t code,
    mach_msg_type_number_t codeCnt, int *flavor, thread_state_t old_state,
    mach_msg_type_number_t old_stateCnt, thread_state_t new_state,
    mach_msg_type_number_t *new_stateCnt)
{
    FuncPutsIDs("beginning");
    return KERN_FAILURE;
}

#endif // END of #if (defined(powerpc) || defined(__powerpc__))

kern_return_t
repair_instruction(mach_port_t victim)
{
    kern_return_t      kr;
    unsigned int       count;
#if (defined(powerpc) || defined(__powerpc__))
    ppc_thread_state_t state;

    FuncPutsIDs("fixing instruction");

    count = MACHINE_THREAD_STATE_COUNT;
    kr = thread_get_state(victim,                 // target thread
                          MACHINE_THREAD_STATE,   // flavor of state to get
                          (thread_state_t)&state, // state information
                          &count);                // in/out size
    EXIT_ON_MACH_ERROR("thread_get_state", kr);

    // SRR0 is used to save the address of the instruction at which execution
    // continues when rfid executes at the end of an exception handler routine
    state.srr0 = (vm_address_t)graceful_dead;
    kr = thread_set_state(victim,                      // target thread
                          MACHINE_THREAD_STATE,        // flavor of state to set
                          (thread_state_t)&state,      // state information
                          MACHINE_THREAD_STATE_COUNT); // in size
    EXIT_ON_MACH_ERROR("thread_set_state", kr);

#elif (defined(x86_64) || defined(__x86_64__))
    x86_thread_state_t state;

    FuncPutsIDs("fixing instruction");

    count = x86_THREAD_STATE_COUNT;
    kr = thread_get_state(victim,                 // target thread
                          x86_THREAD_STATE,   // flavor of state to get
                          (thread_state_t)&state, // state information
                          &count);                // in/out size
    EXIT_ON_MACH_ERROR("thread_get_state", kr);

    printf(L_MARGIN "rip %p (task %#lx, thread %#lx)\n",
            __FUNCTION__, (void*)state.uts.ts64.__rip, (long)mach_task_self(),
            (long)pthread_mach_thread_np(pthread_self()));
    // skip the instrunction by disassembly engine capstone
    int badInstructionLength = instruction_length_at_address(state.uts.ts64.__rip);
    if (-1 == badInstructionLength) {
        FuncPutsIDs("failed to get bad instruction length.")
        return KERN_FAILURE;
    }
    //skip the bad instruciton
    state.uts.ts64.__rip += badInstructionLength;
    printf(L_MARGIN "skip to rip %p (task %#lx, thread %#lx)\n",
            __FUNCTION__, (void*)state.uts.ts64.__rip, (long)mach_task_self(),
            (long)pthread_mach_thread_np(pthread_self()));

    kr = thread_set_state(victim,                      // target thread
                          x86_THREAD_STATE,            // flavor of state to set
                          (thread_state_t)&state,      // state information
                          x86_THREAD_STATE_COUNT);     // in size
    EXIT_ON_MACH_ERROR("thread_set_state", kr);

#elif (defined(aarch64) || defined(__aarch64__))
    arm_thread_state64_t state;

    FuncPutsIDs("fixing instruction");

    count = ARM_THREAD_STATE64_COUNT;
    kr = thread_get_state(victim,                   // target thread
                          ARM_THREAD_STATE64,       // flavor of state to get
                          (thread_state_t)&state,   // state information
                          &count);                  // in/out size
    EXIT_ON_MACH_ERROR("thread_get_state", kr);

    uint64_t pc = arm_thread_state64_get_pc(state);

    printf(L_MARGIN "CPSR %u, PC %p (task %#lx, thread %#lx)\n",
            __FUNCTION__, state.__cpsr, (void*)pc, (long)mach_task_self(),
            (long)pthread_mach_thread_np(pthread_self()));

    /*
    AARCH64 has 4 bytes fixed size instruction set.
    AARCH64 PC register points to the current excuting instruction, while X86-64
    RIP/EIP reigster points to the next instruction

    In the case below, simply applying PC += 4 to skip the instruction is a wrong choice.

    Source:

    main()
    ...
    FuncPuts("about to call function_with_bad_instruction");
    function_with_bad_instruction();
    FuncPuts("after function_with_bad_instruction");
    ...

    Disassemble:

    0x100002808 <+464>: add    x8, x8, #0xcbf            ; "about to call function_with_bad_instruction"
    0x10000280c <+468>: str    x8, [x9, #0x8]
    0x100002810 <+472>: bl     0x100003b90               ; symbol stub for: printf
    0x100002814 <+476>: ldr    x8, [sp, #0x28]
    0x100002818 <+480>: ldr    x8, [x8]
->  0x10000281c <+484>: blr    x8
    0x100002820 <+488>: ldr    x0, [sp, #0x30]
    0x100002824 <+492>: ldr    x8, [sp, #0x38]
    0x100002828 <+496>: mov    x9, sp
    0x10000282c <+500>: str    x8, [x9]
    0x100002830 <+504>: adrp   x8, 1
    0x100002834 <+508>: add    x8, x8, #0xceb            ; "after function_with_bad_instruction"

    When PC = 0x0x10000281c, it is about to invoke function call function_with_bad_instruction.
    The register x8 = 0x000000016fe87000. It is going to jump to the date section. Thus, an exception will be
    thrown. Keep moving.

(lldb) si
Process 9830 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = instruction step into
    frame #0: 0x000000016fe87000
->  0x16fe87000: .long  0xfdc9d8ca                ; unknown opcode
    0x16fe87004: .long  0xa1135cea                ; unknown opcode
    0x16fe87008: udf    #0x0
    0x16fe8700c: udf    #0x0

    On the other hand, the link register LR (X30) keeps the returning address.

    lr = 0x0000000100002820  exception.o`main + 488 at exception.c:145:5

    PC should be set to X30, instead.
    */
    arm_thread_state64_set_pc_fptr(state, pc + 4);
    kr = thread_set_state(victim,                      // target thread
                          ARM_THREAD_STATE64,          // flavor of state to set
                          (thread_state_t)&state,      // state information
                          ARM_THREAD_STATE64_COUNT);   // in size
    EXIT_ON_MACH_ERROR("thread_set_state", kr);

#endif

    FuncPutsIDs("fixed instruction and return KERN_SUCCESS")
    return KERN_SUCCESS;
}
   
void
graceful_dead(void)
{
    FuncPutsIDs("dying graceful death");
}

#if (defined(aarch64) || defined(__aarch64__) || defined(x86_64) || defined(__x86_64__))
int instruction_length_at_address(uint64_t address) {
  csh handle;
  cs_insn *insn;
  size_t count;
  int retval = 0;

#if (defined(x86_64) || defined(__x86_64__))
  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
    return -1;
#elif (defined(aarch64) || defined(__aarch64__))
  if (cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle) != CS_ERR_OK)
    return -1;
#endif // end if (defined(x86_64) || defined(__x86_64__))
  count = cs_disasm(handle, (uint8_t *)address, 64, address, 0, &insn);
  if (count > 0) {
    retval = insn[0].size;
    for (int i = 0; i < count; i++) {
        printf(L_MARGIN "Instruction %d:0x%"PRIx64":\t%s\t\t%s (task %#lx, thread %#lx)\n",
                __FUNCTION__, i+1, insn[i].address, insn[i].mnemonic, insn[i].op_str, (long)mach_task_self(),
                (long)pthread_mach_thread_np(pthread_self()));
    }
    cs_free(insn, count);
  }
  cs_close(&handle);
  return retval;
}
#endif
