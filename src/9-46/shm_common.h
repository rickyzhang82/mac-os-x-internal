// shm_common.h
   
#ifndef _SHM_COMMON_H_
#define _SHM_COMMON_H_
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
   
#define CHECK_ARGS(count, msg) {                          \
    if (argc != count) {                                  \
        fprintf(stderr, "usage: %s " msg "\n", PROGNAME); \
        exit(1);                                          \
    }                                                     \
}
#endif
