// traptest.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
   
extern kern_return_t my_pid_for_task(mach_port_t, int *);
   
int
main(void)
{
    pid_t         pid;
    kern_return_t kr;
    mach_port_t   myTask;
   
    myTask = mach_task_self();
   
    // call the regular trap
    kr = pid_for_task(myTask, (int *)&pid);
    if (kr != KERN_SUCCESS)
        mach_error("pid_for_task:", kr);
    else
        printf("pid_for_task says %d\n", pid);
   
    // call our version of the trap
    kr = my_pid_for_task(myTask, (int *)&pid);
    if (kr != KERN_SUCCESS)
        mach_error("my_pid_for_task:", kr);
    else
        printf("my_pid_for_task says %d\n", pid);
   
    exit(0);
}
