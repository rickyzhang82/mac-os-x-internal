#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
   
#define PROGNAME "callfunc"
   
typedef void (*func_t)(void);
   
int
main(int argc, char **argv)
{
    unsigned long long addr;
    func_t func;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <address in hexadecimal>\n", PROGNAME);
        exit(1);
    }
   
    addr = strtoull(argv[1], NULL, 16);
    if (!addr || (addr == ULLONG_MAX)) {
        perror("strtoull");
        exit(1);
    }
   
    func = (func_t)(uintptr_t)addr;
    func();
   
    return 0;
}
