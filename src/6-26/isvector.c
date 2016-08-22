// isvector.c
   
#include <stdio.h>
   
// defined in osfmk/ppc/thread_act.h
#define vectorUsed 0x20000000
#define floatUsed  0x40000000
#define runningVM  0x80000000
   
extern int my_facstat(void);
   
int
main(int argc, char **argv)
{
    int facstat;
    vector signed int c;
   
    if (argc > 1)
        c  = (vector signed int){ 1, 2, 3, 4 };
   
    facstat = my_facstat();
   
    printf("%s\n", (facstat & vectorUsed) ? \
           "vector used" : "vector not used");
   
    return 0;
}
