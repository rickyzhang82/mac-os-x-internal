// CHUDCall.c
   
#include <stdio.h>
   
int
CHUDCall(void)
{
    int ret;
   
    __asm__ volatile(
        "li r0,0x6009\n"
        "sc\n"
        "mr %0,r3\n"
        : "=r" (ret) // output
        :            // no input
    );
   
    return ret;
}
   
int
main(void)
{
    int ret = CHUDCall();
   
    printf("%d\n", ret);
   
    return ret;
}
