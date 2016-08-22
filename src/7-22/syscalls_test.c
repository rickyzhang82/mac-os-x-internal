// syscalls_test.c
   
#include <stdio.h>
#include <fcntl.h>>
#include <unistd.h>
#include <mach/mach.h>
   
int
main()
{
    int           i, fd;
    mach_port_t   p;
    kern_return_t kr;
   
    setbuf(stdout, NULL);
    printf("My pid is %d\n", getpid());
    printf("Note the number of Mach and Unix system calls, and press <enter>");
    (void)getchar();
   
    // At this point, we will have some base numbers of Mach and Unix
    // system calls made so far, say, M and U, respectively
   
    for (i = 0; i < 100; i++) { // 100 iterations
   
        // +1 Unix system call per iteration
        fd = open("/dev/null", O_RDONLY);
   
        // +1 Unix system call per iteration
        close(fd);
   
        // +1 Mach system call per iteration
        kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &p);
   
        // +1 Mach system call per iteration
        kr = mach_port_deallocate(mach_task_self(), p);
   
    }
   
    // +1 Unix system call
    printf("Note the number of Mach and Unix system calls again...\n"
           "Now sleeping for 60 seconds...");
   
    // sleep(3) is implemented using nanosleep, which will call
    // clock_get_time() and clock_sleep_trap() -- this is +2 Mach system calls
   
    (int)sleep(60);
   
    // Mach system calls = M + 2 * 100 + 2 (that is, 202 more calls)
    // Unix system calls = U + 2 * 100 + 1 (that is, 201 more calls)
   
    return 0;
}
