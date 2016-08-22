// sem_common.h
   
#ifndef _SEM_COMMON_H_
#define _SEM_COMMON_H_
   
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
   
#define CHECK_ARGS(count, msg) {                          \
    if (argc != count) {                                  \
        fprintf(stderr, "usage: %s " msg "\n", PROGNAME); \
        exit(1);                                          \
    }                                                     \
}
#endif
