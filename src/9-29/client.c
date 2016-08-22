// client.c
   
#include "misc_types.h"
   
#define INPUT_STRING "Hello, MIG!"
#define INPUT_NUMBER 5
   
int
main(int argc, char **argv)
{
    kern_return_t kr;
    mach_port_t   server_port;
    int           len, fac;
   
    // look up the service to find the server's port
    if ((kr = bootstrap_look_up(bootstrap_port, MIG_MISC_SERVICE,
                                &server_port)) != BOOTSTRAP_SUCCESS) {
        mach_error("bootstrap_look_up:", kr);
        exit(1);
    }
   
    // call a procedure
    if ((kr = string_length(server_port, INPUT_STRING, &len)) != KERN_SUCCESS)
        mach_error("string_length:", kr);
    else
        printf("length of \"%s\" is %d\n", INPUT_STRING, len);
   
    // call another procedure
    if ((kr = factorial(server_port, INPUT_NUMBER, &fac)) != KERN_SUCCESS)
        mach_error("factorial:", kr);
    else
        printf("factorial of %d is %d\n", INPUT_NUMBER, fac);
   
    mach_port_deallocate(mach_task_self(), server_port);
   
    exit(0);
}
