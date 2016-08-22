// traceme.c
   
#include <stdlib.h>

#if defined(__GNUC__)
#include <ppc_intrinsics.h>
#endif
   
int
main(void)
{
    int i, a = 0;
   
    // supervisor-level instruction as a trigger
    // start tracing
    (void)__mfspr(1023);
   
    for (i = 0; i < 16; i++) {
        a += 3 * i;
    }
   
    // supervisor-level instruction as a trigger
    // stop tracing
    (void)__mfspr(1023);
   
   exit(0);
}
