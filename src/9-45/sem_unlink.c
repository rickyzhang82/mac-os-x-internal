// sem_unlink.c
   
#include "sem_common.h"
   
#define PROGNAME "sem_unlink"
   
int
main(int argc, char **argv)
{
    int ret = 0;
   
    CHECK_ARGS(2, "<path>");
   
    if ((ret = sem_unlink(argv[1])) < 0)
        perror("sem_unlink");
   
    exit(ret);
}
