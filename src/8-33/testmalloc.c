// test_malloc.c (32-bit)
   
#include <stdlib.h>
   
int watch = -1;
   
int
main(void)
{
    void *ptr1, *ptr2, *ptr3, *ptr4;
   
    ptr1 = malloc(490); // 31 TINY_QUANTUMs
    ptr2 = malloc(491); // 31 TINY_QUANTUMs
    ptr3 = malloc(492); // 31 TINY_QUANTUMs
    ptr4 = malloc(493); // 31 TINY_QUANTUMs
    watch = 1;          // breakpoint here
   
    free(ptr1);
    free(ptr3);
    watch = 2;          // breakpoint here
   
    free(ptr2);
    free(ptr4);
    watch = 3;          // breakpoint here
   
    return 0;
}
