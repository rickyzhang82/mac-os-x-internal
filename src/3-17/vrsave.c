// vrsave.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
   
void prbits(u_int32_t);
u_int32_t read_vrsave(void);
   
// Print the bits of a 32-bit number
void
prbits32(u_int32_t u)
{
    u_int32_t i = 32;
   
    for (; i--; putchar(u & 1 << i ? '1' : '0'));
   
    printf("\n");
}
   
// Retrieve the contents of the VRSAVE
u_int32_t
read_vrsave(void)
{
    u_int32_t v;
   
    __asm("mfspr %0,VRsave\n\t"
          : "=r"(v)
          :
    );
   
    return v;
}
   
int
main()
{
    vector float v1, v2, v3;
   
    v1 = (vector float)(1.0, 2.0, 3.0, 4.0);
    v2 = (vector float)(2.0, 3.0, 4.0, 5.0);
   
    v3 = vec_add(v1, v2);
   
    prbits32(read_vrsave());
   
    exit(0);
}
