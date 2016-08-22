// shm_unlink.c
   
#include "shm_common.h"
   
#define PROGNAME "shm_unlink"
   
int
main(int argc, char **argv)
{
    int ret = 0;
   
    CHECK_ARGS(2, "<path>");
   
    if ((ret = shm_unlink(argv[1])))
        perror("shm_unlink");
   
    exit(ret);
}
