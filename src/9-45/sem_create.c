// sem_create.c
   
#include "sem_common.h"
   
#define PROGNAME "sem_create"
   
int
main(int argc, char **argv)
{
    int    val;
    sem_t *sem;
   
    CHECK_ARGS(3, "<path> <value>");
   
    val = atoi(argv[2]);
   
    sem = sem_open(argv[1], O_CREAT | O_EXCL, 0644, val);
    if (sem == (sem_t *)SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
   
    sem_close(sem);
   
    exit(0);
}
