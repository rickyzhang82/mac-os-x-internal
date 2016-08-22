// getpid_demo.c
   
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
   
pid_t
my_getpid(void)
{
    int syscallnum = SYS_getpid;
   
    __asm__ volatile(
        "lwz r0,%0\n"
        "sc\n"
        "nop\n" // The kernel will arrange for this to be skipped
      :
      : "g" (syscallnum)
    );
   
    // GPR3 already has the right return value
    // Compiler warning here because of the lack of a return statement
}
   
int
main(void)
{
    printf("my pid is %d\n", getpid());
    printf("my pid is %d\n", my_getpid());
   
    return 0;
}
