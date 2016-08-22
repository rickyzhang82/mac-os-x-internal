#include <stdio.h>
#include <pthread.h>
   
extern pthread_t my_pthread_self();
extern pthread_t my_pthread_self_970();
   
int
main(void)
{
    printf("library: %p\n", pthread_self());        // call library function
    printf("UFT    : %p\n", my_pthread_self());     // use 0x7FF2 UFT
    printf("SPRG3  : %p\n", my_pthread_self_970()); // read from SPRG3
   
    return 0;
}
