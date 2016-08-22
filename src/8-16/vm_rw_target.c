// vm_rw_target.c
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
   
#define SOME_CHAR 'A'
   
int
main()
{
    kern_return_t     kr;
    mach_vm_address_t address;
    mach_vm_size_t    size = (mach_vm_size_t)vm_page_size;
   
    // get a page of memory
    kr = mach_vm_allocate(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS) {
        mach_error("vm_allocate:", kr);
        exit(1);
    }
   
    // color it with something
    memset((char *)address, SOME_CHAR, vm_page_size);
   
    // display the address so the master can read/write to it
    printf("pid=%d, address=%p\n", getpid(), (void *)address);
   
    // wait until master writes to us
    while (*(char *)address == SOME_CHAR)
        ;
   
    mach_vm_deallocate(mach_task_self(), address, size);
   
    exit(0);
}
