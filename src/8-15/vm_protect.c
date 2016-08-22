// vm_protect.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
int
main(int argc, char **argv)
{
    char             *ptr;
    kern_return_t     kr;
    mach_vm_address_t a_page = (mach_vm_address_t)0;
    mach_vm_size_t    a_size = (mach_vm_size_t)vm_page_size;
   
    kr = mach_vm_allocate(mach_task_self(), &a_page, a_size, VM_FLAGS_ANYWHERE);
    OUT_ON_MACH_ERROR("vm_allocate", kr);
   
    ptr = (char *)a_page + 2048;
   
    snprintf(ptr, (size_t)16, "Hello, Mach!");
   
    if (argc == 2) { // deny read access to a_page
        kr = mach_vm_protect(
                 mach_task_self(),         // target address space
                 (mach_vm_address_t)a_page,// starting address of region
                 (mach_vm_size_t)4,        // length of region in bytes
                 FALSE,                    // set maximum?
                 VM_PROT_NONE);            // deny all access
        OUT_ON_MACH_ERROR("vm_protect", kr);
    }
   
    printf("%s\n", ptr);
   
out:
    if (a_page)
        mach_vm_deallocate(mach_task_self(), a_page, a_size);
   
    exit(kr);
}
