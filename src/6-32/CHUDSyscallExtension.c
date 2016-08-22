// CHUDSyscallExtension.c
   
#include <sys/systm.h>
#include <mach/mach_types.h>
   
#define XNU_KERNEL_PRIVATE
#define __APPLE_API_PRIVATE
#define MACH_KERNEL_PRIVATE
   
// Either include the appropriate headers or provide structure declarations
// for the following:
//
// struct savearea
// struct ppc_thread_state
// struct ppc_thread_state64
   
// PowerPC-only system call table (from osfmk/ppc/PPCcalls.h)
typedef int (* PPCcallEnt)(struct savearea *save);
extern PPCcallEnt PPCcalls[];
   
// The callback function's prototype
typedef kern_return_t (* ppc_syscall_callback_func_t) \
                      (thread_flavor_t flavor, thread_state_t tstate, \
                       mach_msg_type_number_t count);
   
// Pointer for referring to the incoming callback function
static ppc_syscall_callback_func_t callback_func = NULL;
   
// Identical to chudxnu_copy_savearea_to_threadstate(), which is implemented
// in osfmk/ppc/chud/chud_osfmk_callbacks.c
kern_return_t
ppc_copy_savearea_to_threadstate(thread_flavor_t         flavor,
                                 thread_state_t          tstate,
                                 mach_msg_type_number_t *count,
                                 struct savearea        *sv)
{
    ...
}
   
// PPCcalls[9] will point to this when a callback is registered
kern_return_t
callback_wrapper(struct savearea *ssp)
{
    if (ssp) {
        if (callback_func) {
            struct my_ppc_thread_state64 state;
            mach_msg_type_number_t       count = PPC_THREAD_STATE64_COUNT;
   
            ppc_copy_savearea_to_threadstate(PPC_THREAD_STATE64,
                                             (thread_state_t)&state,
                                             &count, ssp);
   
            ssp->save_r3 = (callback_func)(PPC_THREAD_STATE64,
                                           (thread_state_t)&state, count);
        } else {
            ssp->save_r3 = KERN_FAILURE;
        }
    }
        
    return 1; // Check for ASTs
}
   
// Example callback function
kern_return_t
callback_func_example(thread_flavor_t        flavor,
                      thread_state_t         tstate,
                      mach_msg_type_number_t count)
{
    printf("Hello, CHUD!\n");
    return KERN_SUCCESS;
}
   
// Callback registration
kern_return_t
ppc_syscall_callback_enter(ppc_syscall_callback_func_t func)
{
    callback_func = func;
    PPCcalls[9] = callback_wrapper;
    __asm__ volatile("eieio");
    __asm__ volatile("sync");
    return KERN_SUCCESS;
}
   
// Callback cancellation
kern_return_t
ppc_syscall_callback_cancel(void)
{
    callback_func = NULL;
    PPCcalls[9] = NULL;
    __asm__ volatile("eieio");
    __asm__ volatile("sync");
    return KERN_SUCCESS;
}
   
kern_return_t
PPCSysCallKEXT_start(kmod_info_t *ki, void *d)
{
    ppc_syscall_callback_enter(callback_func_example);
    printf("PPCSysCallKEXT_start\n");
    return KERN_SUCCESS;
}
   
kern_return_t
PPCSysCallKEXT_stop(kmod_info_t *ki, void *d)
{
    ppc_syscall_callback_cancel();
    printf("PPCSysCallKEXT_stop\n");
    return KERN_SUCCESS;
}
