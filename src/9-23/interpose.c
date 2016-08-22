// interpose.c
   
#include <stdio.h>
#include <unistd.h>
#include <mach/mach.h>
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
void
print_processor_count(host_priv_t host_priv)
{
    kern_return_t          kr;
    natural_t              processor_count = 0;
    processor_port_array_t processor_list;
   
    kr = host_processors(host_priv, &processor_list, &processor_count);
    if (kr == KERN_SUCCESS)
        printf("%d processors\n", processor_count);
    else
        mach_error("host_processors:", kr);
}
   
void
childproc()
{
    printf("child suspending...\n");
    (void)task_suspend(mach_task_self());
    printf("child attempting to retrieve processor count...\n");
    print_processor_count(0x1234);
}
   
void
parentproc(pid_t child)
{
    kern_return_t kr;
    task_t        child_task;
    host_priv_t   host_priv;
   
    // kludge: give child some time to run and suspend itself
    sleep(1);
   
    kr = task_for_pid(mach_task_self(), child, &child_task);
    OUT_ON_MACH_ERROR("task_for_pid", kr);
   
    kr = host_get_host_priv_port(mach_host_self(), &host_priv);
    OUT_ON_MACH_ERROR("host_get_host_priv_port", kr);
   
    kr = mach_port_insert_right(child_task, 0x1234, host_priv,
                                MACH_MSG_TYPE_MOVE_SEND);
    if (kr != KERN_SUCCESS)
        mach_error("mach_port_insert_right:", kr);
   
out:
    printf("resuming child...\n");
    (void)task_resume(child_task);
}
   
int
main(void)
{
    pid_t pid = fork();
   
    if (pid == 0)
        childproc();
    else if (pid > 0)
        parentproc(pid);
    else
        return 1;
   
    return 0;
}
