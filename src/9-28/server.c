// server.c
   
#include "misc_types.h"
   
static mach_port_t server_port;
   
extern boolean_t misc_server(mach_msg_header_t *inhdr,
                             mach_msg_header_t *outhdr);
   
void
server_setup(void)
{
    kern_return_t kr;
   
   if ((kr = bootstrap_create_service(bootstrap_port, MIG_MISC_SERVICE,
                                      &server_port)) != BOOTSTRAP_SUCCESS) {
        mach_error("bootstrap_create_service:", kr);
        exit(1);
    }
   
    if ((kr = bootstrap_check_in(bootstrap_port, MIG_MISC_SERVICE,
                                 &server_port)) != BOOTSTRAP_SUCCESS) {
        mach_port_deallocate(mach_task_self(), server_port);
        mach_error("bootstrap_check_in:", kr);
        exit(1);
    }
}
   
void
server_loop(void)
{
    mach_msg_server(misc_server,            // call the server-interface module
                    sizeof(msg_misc_t),     // maximum receive size
                    server_port,            // port to receive on
                    MACH_MSG_TIMEOUT_NONE); // options
}
   
// InTran
xput_number_t
misc_translate_int_to_xput_number_t(int param)
{
     printf("misc_translate_incoming(%d)\n", param);
     return (xput_number_t)param;
}
   
// OutTran
int
misc_translate_xput_number_t_to_int(xput_number_t param)
{
     printf("misc_translate_outgoing(%d)\n", (int)param);
     return (int)param;
}
   
// Destructor
void
misc_remove_reference(xput_number_t param)
{
     printf("misc_remove_reference(%d)\n", (int)param);
}
   
// an operation that we export
kern_return_t
string_length(mach_port_t     server_port,
              input_string_t  instring,
              xput_number_t  *len)
{
    char *in = (char *)instring;
   
    if (!in || !len)
        return KERN_INVALID_ADDRESS;
   
    *len = 0;
   
    while (*in++)
        (*len)++;
   
    return KERN_SUCCESS;
}
   
// an operation that we export
kern_return_t
factorial(mach_port_t server_port, xput_number_t num, xput_number_t *fac)
{
    int i;
   
    if (!fac)
        return KERN_INVALID_ADDRESS;
   
    *fac = 1;
   
    for (i = 2; i <= num; i++)
        *fac *= i;
   
    return KERN_SUCCESS;
}
   
int
main(void)
{
    server_setup();
    server_loop();
    exit(0);
}
