// fork_demo.c
   
#include <unistd.h>
#include <sys/syscall.h>
   
int
main(void)
{
    long r3_r4[] = { -1, -1 };
    int syscallnum = SYS_fork;
   
    __asm__ volatile(
        "lwz r0,%2      ; load GPR0 with SYS_fork\n"
        "sc             ; invoke fork(2)\n"
        "nop            ; this will be skipped in the case of no error\n"
        "mr %0,r3       ; save GPR3 to r3_r4[0]\n"
        "mr %1,r4       ; save GPR4 to r3_r4[1]\n"
      : "=r"(r3_r4[0]), "=r"(r3_r4[1])
      : "g" (syscallnum)
    );
   
    // write GPR3 and GPR4
    write(1, r3_r4, sizeof(r3_r4)/sizeof(char));
   
    // sleep for 30 seconds so we can check process IDs using 'ps'
    sleep(30);
   
    return 0;
}
