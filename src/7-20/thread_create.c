// thread_create.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <architecture/ppc/cframe.h>
   
void my_thread_setup(thread_t t);
void my_thread_exit(void);
void my_thread_routine(int, char *);
   
static uintptr_t threadStack[PAGE_SIZE];
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
int
main(int argc, char **argv)
{
    thread_t         th;
    kern_return_t    kr;
    mach_port_name_t mytask, mythread;
   
    mytask = mach_task_self();
    mythread = mach_thread_self();
   
    // create new thread within our task
    kr = thread_create(mytask, &th);
    EXIT_ON_MACH_ERROR("thread_create", kr);
   
    // set up the new thread's user mode execution state
    my_thread_setup(th);
   
    // run the new thread
    kr = thread_resume(th);
    EXIT_ON_MACH_ERROR("thread_resume", kr);
   
    // new thread will call exit
    // note that we still have an undiscarded reference on mythread
    thread_suspend(mythread);
   
    /* NOTREACHED */
   
    exit(0);
}
   
void
my_thread_setup(thread_t th)
{
    kern_return_t           kr;
    mach_msg_type_number_t  count;
    ppc_thread_state_t      state;
    void                   *stack = threadStack;
   
    // arguments to my_thread_routine() -- the function run by the new thread
    int arg1 = 16;
    char *arg2 = "Hello, Mach!";
   
    stack += (PAGE_SIZE - C_ARGSAVE_LEN - C_RED_ZONE);
   
    count = PPC_THREAD_STATE_COUNT;
    kr = thread_get_state(th,               // target thread
                          PPC_THREAD_STATE, // flavor of thread state
                          (thread_state_t)&state, &count);
    EXIT_ON_MACH_ERROR("thread_get_state", kr);
   
    //// setup of machine-dependent thread state (PowerPC)
   
    state.srr0 = (unsigned int)my_thread_routine; // where to begin execution
    state.r1 = (uintptr_t)stack; // stack pointer
    state.r3 = arg1;             // first argument to my_thread_routine()
    state.r4 = (uintptr_t)arg2;  // second argument to my_thread_routine()
    // "return address" for my_thread_routine()
    state.lr = (unsigned int)my_thread_exit;
   
    kr = thread_set_state(th, PPC_THREAD_STATE, (thread_state_t)&state,
                          PPC_THREAD_STATE_COUNT);
    EXIT_ON_MACH_ERROR("my_thread_setup", kr);
}
   
void
my_thread_routine(int arg1, char *arg2)
{
    // printf("my_thread_routine(%d, %s)\n", arg1, arg2); // likely to fail
    puts("my_thread_routine()");
}
   
void
my_thread_exit(void)
{
    puts("my_thread_exit(void)");
    exit(0);
}
