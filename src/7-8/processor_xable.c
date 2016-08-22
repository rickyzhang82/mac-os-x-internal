// processor_xable.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
#define PROGNAME "processor_xable"
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg, kr); exit((retval)); }
   
int
main(int argc, char **argv)
{
    kern_return_t          kr;
    host_priv_t            host_priv;
    processor_port_array_t processor_list;
    natural_t              processor_count;
    char                  *errmsg = PROGNAME;
   
    if (argc != 2) {
        fprintf(stderr,
                "usage: %s <cmd>, where <cmd> is \"exit\" or \"start\"\n",
                PROGNAME);
        exit(1);
    }
   
    kr = host_get_host_priv_port(mach_host_self(), &host_priv);
    EXIT_ON_MACH_ERROR("host_get_host_priv_port:", kr);   
   
    kr = host_processors(host_priv, &processor_list, &processor_count);
    EXIT_ON_MACH_ERROR("host_processors:", kr);
   
    // disable last processor on a multiprocessor system
    if (processor_count > 1) {
        if (*argv[1] == 'e') {
            kr = processor_exit(processor_list[processor_count - 1]);
            errmsg = "processor_exit:";
        } else if (*argv[1] == 's') {
            kr = processor_start(processor_list[processor_count - 1]);
            errmsg = "processor_start:";
        } else {
            kr = KERN_INVALID_ARGUMENT;
        }
    } else
        printf("Only one processor!\n");
   
    // this will deallocate while rounding up to page size
    (void)vm_deallocate(mach_task_self(), (vm_address_t)processor_list,
                        processor_count * sizeof(processor_t *));
    EXIT_ON_MACH_ERROR(errmsg, kr);
   
    fprintf(stderr, "%s successful\n", errmsg);
   
    exit(0);
}
