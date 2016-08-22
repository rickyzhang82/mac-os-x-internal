// bootstrap_info.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
   
int
main(int argc, char **argv)
{
    kern_return_t            kr;
    name_array_t             service_names, server_names;
    bootstrap_status_array_t service_active;
    unsigned int             service_names_count, server_names_count;
    unsigned int             service_active_count, i;
   
    // We can use bootstrap_port, a global variable declared in a Mach header,
    // for the current task's bootstrap port. Alternatively, we can explicitly
    // retrieve the same send right by calling task_get_bootstrap_port(),
    // specifying mach_task_self() as the target task. This is how the system
    // library initializes the global variable.
   
    // launchd implements this routine
    kr = bootstrap_info(bootstrap_port,
                        &service_names,
                        &service_names_count,
                        &server_names,
                        &server_names_count,
                        &service_active,
                        &service_active_count);
    if (kr != BOOTSTRAP_SUCCESS) {
        mach_error("bootstrap_info:", kr);
        exit(1);
    }
   
    printf("%s %-48s %s\n%s %-48s %s\n", "up?", "service name", "server cmd",
           "___", "____________", "__________");
   
    for (i = 0; i < service_names_count; i++)
        printf("%s %-48s %s\n",
               (service_active[i]) ? "1  " : "0  ", service_names[i],
               (server_names[i][0] == '\0') ? "-" : server_names[i]);
   
    // The service_names, server_names, and service_active arrays have been
    // vm_allocate()'d in our address space. Both "names" arrays are of type
    // name_array_t, which is an array of name_t elements. A name_t in turn
    // is a character string of length 128.
    //
    // As good programming practice, we should call vm_deallocate() to free up
    // such virtual memory when it is not needed anymore.
   
    (void)vm_deallocate(mach_task_self(), (vm_address_t)service_active,
                        service_active_count * sizeof(service_active[0]));
   
    (void)vm_deallocate(mach_task_self(), (vm_address_t)service_names,
                        service_names_count * sizeof(service_names[0]));
   
    (void)vm_deallocate(mach_task_self(), (vm_address_t)server_names,
                        server_names_count * sizeof(server_names[0]));
   
    exit(0);
}
