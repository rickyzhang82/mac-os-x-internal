// altivec.c
   
#include <stdio.h>
#include <stdlib.h>
   
int
main(void)
{
    // "vector" is an AltiVec keyword
    vector float v1, v2, v3;
    
    v1 = (vector float)(1.0, 2.0, 3.0, 4.0);
    v2 = (vector float)(2.0, 3.0, 4.0, 5.0);
    
    // vector_add() is a compiler built-in function
    v3 = vector_add(v1, v2);
    
    // "%vf" is a vector-formatting string for printf()
    printf("%vf\n", v3);
    
    exit(0);
}
