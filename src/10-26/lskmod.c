// lskmod.c
   
#include <stdio.h>
#include <mach/mach.h>
   
int
main(void)
{
    kern_return_t           kr;
    kmod_info_array_t       kmods;
    mach_msg_type_number_t  kmodBytes = 0;
    int                     kmodCount = 0;
    kmod_info_t            *kmodp;
    mach_port_t             host_port = mach_host_self();
   
    kr = kmod_get_info(host_port, (void *)&kmods, &kmodBytes);
    (void)mach_port_deallocate(mach_task_self(), host_port);
    if (kr != KERN_SUCCESS) {
        mach_error("kmod_get_info:", kr);
        return kr;
    }
   
    for (kmodp = (kmod_info_t *)kmods; kmodp->next; kmodp++, kmodCount++) {
        printf("%5d %4d %-10p %-10p %-10p %s (%s)\n",
               kmodp->id,
               kmodp->reference_count,
               (void *)kmodp->address, 
               (void *)kmodp->size,
               (void *)(kmodp->size - kmodp->hdr_size),
               kmodp->name,
               kmodp->version);
    }
   
    vm_deallocate(mach_task_self(), (vm_address_t)kmods, kmodBytes);
   
    return kr;
}
