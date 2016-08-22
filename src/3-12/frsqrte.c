// frsqrte.c
   
#include <stdio.h>
#include <stdlib.h>
   
double
frsqrte(double n)
{
    double s;
   
    asm(
        "frsqrte %0, %1"
        : "=f" (s)  /* out */
        : "f"  (n)  /* in */
    );
   
    return s;
}
   
int
main(int argc, char **argv)
{
    printf("%8.8f\n", frsqrte(strtod(argv[1], NULL)));
    return 0;
}
