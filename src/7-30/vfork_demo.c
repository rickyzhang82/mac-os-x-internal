// vfork_demo.c
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <mach/mach.h>
   
int
main(int argc, char **argv)
{
    int ret, i = 0;
   
    printf("parent: task = %x, thread = %x\n", mach_task_self(),
           pthread_mach_thread_np(pthread_self()));
   
    // vfork() if no extra command-line arguments
    if (argc == 1)
        ret = vfork();
    else
        ret = fork();
   
    if (ret < 0)
        exit(ret);
   
    if (ret == 0) { // child
        i = 1;
        printf("child: task = %x, thread = %x\n", mach_task_self(),
               pthread_mach_thread_np(pthread_self()));
        _exit(0);
    } else
        printf("parent, i = %d\n", i);
   
    return 0;
}
