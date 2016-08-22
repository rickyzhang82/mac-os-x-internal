// shm_create.c
   
#include "shm_common.h"
#include <string.h>
   
#define PROGNAME "shm_create"
   
int
main(int argc, char **argv)
{
    char   *p;
    int     shm_fd;
    size_t  len;
   
    CHECK_ARGS(3, "<path> <shared string>");
   
    if ((shm_fd = shm_open(argv[1], O_CREAT | O_EXCL | O_RDWR, S_IRWXU)) < 0) {
        perror("shm_open");
        exit(1);
    }
   
    len = strlen(argv[2]) + 1;
    ftruncate(shm_fd, len);
    if (!(p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0))) {
        perror("mmap");
        shm_unlink(argv[1]);
        exit(1);
    }
   
    // copy the user-provided data into the shared memory
    snprintf(p, len + 1, "%s", argv[2]);
    munmap(p, len);
   
    close(shm_fd);
   
    exit(0);
}
