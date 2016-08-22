// sem_post.c
   
#include "sem_common.h"
   
#define PROGNAME "sem_post"
   
int
main(int argc, char **argv)
{
    int    ret = 0;
    sem_t *sem;
   
    CHECK_ARGS(2, "<path>");
   
    sem = sem_open(argv[1], 0);
    if (sem == (sem_t *)SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
   
    if ((ret = sem_post(sem)) < 0)
        perror("sem_post");
   
    sem_close(sem);
   
    exit(ret);
}
