#include <stdio.h>
   
extern void weakfunc(void) __attribute__((weak_import));
   
int
main(void)
{
    if (weakfunc)
        weakfunc();
    else
        puts("Weak function not found.");
   
    return 0;
}
