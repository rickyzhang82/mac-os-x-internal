// DummySysctl.c
   
#include <mach/mach_types.h>
   
kern_return_t
DummySysctl_start(kmod_info_t *ki, void *d)
{
    printf("DummySysctl_start\n");
    return KERN_SUCCESS;
}
   
kern_return_t
DummySysctl_stop(kmod_info_t *ki, void *d)
{
    printf("DummySysctl_stop\n");
    return KERN_SUCCESS;
}
