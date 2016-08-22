// cntlzd_main.c
   
#include <stdio.h>
#include <stdint.h>
   
extern uint64_t cntlzd(uint64_t in);
   
int
main(void)
{
    uint64_t out;
    uint64_t in = 0x4000000000000000LL;
   
    out = cntlzd(in);
   
    printf("%lld\n", out);
   
    __asm("cntlzd %0,%1\n"
          : "=r"(out)
          :  "r"(in)
    );
   
    printf("%lld\n", out);
   
    return 0;
}
