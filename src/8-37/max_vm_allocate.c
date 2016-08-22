// max_vm_allocate.c
   
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
   
#define PROGNAME "max_vm_allocate"
   
int
main(int argc, char **argv)
{
    kern_return_t      kr;
    unsigned long long nbytes;
    mach_vm_address_t  address;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <number of bytes>\n", PROGNAME);
        exit(1);
    }
   
    if ((nbytes = strtoull(argv[1], NULL, 10)) == ULLONG_MAX) {
        fprintf(stderr, "invalid number of bytes specified\n");
        exit(1);
    }
   
    kr = mach_vm_allocate(mach_task_self(), &address,
                          (mach_vm_size_t)nbytes, TRUE);
    if (kr == KERN_SUCCESS) {
        printf("allocated %llu bytes at %p\n", nbytes, (void *)address);
        mach_vm_deallocate(mach_task_self(), address, (mach_vm_size_t)nbytes);
    } else
        mach_error("mach_vm_allocate:", kr);
   
    exit(0);
}
