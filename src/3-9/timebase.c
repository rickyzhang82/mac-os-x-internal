// timebase.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
   
u_int64_t mftb64(void);
void mftb32(u_int32_t *, u_int32_t *);
   
int
main(void)
{
    u_int64_t tb64;
    u_int32_t tb32u, tb32l;
   
    tb64 = mftb64();
    mftb32(&tb32u, &tb32l);
   
    printf("%llx %x%08x\n", tb64, tb32l, tb32u);
    exit(0);
}
   
// Requires a 64-bit processor
// The TBR can be read in a single instruction (TBU || TBL)
u_int64_t
mftb64(void)
{
    u_int64_t tb64;
   
    __asm("mftb %0\n\t"
          : "=r" (tb64)
          :
    );
   
    return tb64;
}
   
// 32-bit or 64-bit
void
mftb32(u_int32_t *u, u_int32_t *l)
{
    u_int32_t tmp;
   
    __asm(
    "loop:              \n\t"
        "mftbu    %0    \n\t"
        "mftb     %1    \n\t"
        "mftbu    %2    \n\t"
        "cmpw     %2,%0 \n\t"
        "bne      loop  \n\t"
        : "=r"(*u), "=r"(*l), "=r"(tmp)
        :
    );
}
