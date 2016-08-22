// large_malloc.c
   
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mach/mach.h>
   
#define PROGNAME "large_malloc"
   
int
main(int argc, char **argv)
{
    void *ptr;
    unsigned long long npages;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <allocation size in pages>\n", PROGNAME);
        exit(1);
    }
   
    if ((npages = strtoull(argv[1], NULL, 10)) == ULLONG_MAX) {
        perror("strtoull");
        exit(1);
    }
   
    if ((ptr = malloc((size_t)(npages << vm_page_shift))) == NULL)
        perror("malloc");
    else
        free(ptr);
   
    exit(0);
}
