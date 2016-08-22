// bootstrap_server.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <asl.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
   
#define SERVICE_NAME          "com.osxbook.DummySleeper"
#define SERVICE_CMD           "/tmp/sleeperd"
#define SERVICE_SHUTDOWN_FILE SERVICE_CMD ".off"
   
static mach_port_t server_priv_port;
static aslmsg      logmsg;
   
// Note that asl_log() accepts the %m formatting character, which is
// replaced by the ASL facility with the error string corresponding to
// the errno variable's current value.
#define MY_ASL_LOG(fmt, ...) \
    asl_log(NULL, logmsg, ASL_LEVEL_ERR, fmt, ## __VA_ARGS__)
   
static kern_return_t
register_bootstrap_service(void)
{
    kern_return_t kr;
    mach_port_t   service_send_port, service_rcv_port;
   
    // Let us attempt to check in.... This routine will look up the service
    // by name and attempt to return receive rights to the service port.
    kr = bootstrap_check_in(bootstrap_port, (char *)SERVICE_NAME,
                            &service_rcv_port);
    if (kr == KERN_SUCCESS)
        server_priv_port = bootstrap_port;
    else if (kr == BOOTSTRAP_UNKNOWN_SERVICE) {
   
        // The service does not exist, so let us create it....
   
        kr = bootstrap_create_server(bootstrap_port, 
                                     SERVICE_CMD,
                                     getuid(),       // server uid
                                     FALSE,          // not on-demand
                                     &server_priv_port);
       if (kr != KERN_SUCCESS)
           return kr;
   
       // We can now use server_priv_port to declare services associated
       // with this server by calling bootstrap_create_service() and passing
       // server_priv_port as the bootstrap port.
   
       // Create a service called SERVICE_NAME, and return send rights to
       // that port in service_send_port.
       kr = bootstrap_create_service(server_priv_port, (char *)SERVICE_NAME,
                                     &service_send_port);
       if (kr != KERN_SUCCESS) {
           mach_port_deallocate(mach_task_self(), server_priv_port);
           return kr;
       }
   
       // Check in and get receive rights to the service port of the service.
       kr = bootstrap_check_in(server_priv_port, (char *)SERVICE_NAME,
                               &service_rcv_port);
       if (kr != KERN_SUCCESS) {
           mach_port_deallocate(mach_task_self(), server_priv_port);
           mach_port_deallocate(mach_task_self(), service_send_port);
           return kr;
       }
    }
   
    // We are not a Mach port server, so we do not need this port. However,
    // we still will have a service with the Bootstrap Server, and so we
    // will be relaunched if we exit.
    mach_port_destroy(mach_task_self(), service_rcv_port);
   
    return kr;
}
   
static kern_return_t
unregister_bootstrap_service(void)
{
    return bootstrap_register(server_priv_port, (char *)SERVICE_NAME,
                              MACH_PORT_NULL);
}
   
int
main(void)
{
    kern_return_t kr;
    struct stat   statbuf;
   
    // Initialize a message for use with the Apple System Log (asl) facility.
    logmsg = asl_new(ASL_TYPE_MSG);
    asl_set(logmsg, "Facility", "Sleeper Daemon");
   
    // If the shutdown flag file exists, we are destroying the service;
    // otherwise, we are trying to be a server.
    if (stat(SERVICE_SHUTDOWN_FILE, &statbuf) == 0) {
        kr = unregister_bootstrap_service();
        MY_ASL_LOG("destroying service %s\n", SERVICE_NAME);
    } else {
        kr = register_bootstrap_service();
        MY_ASL_LOG("starting up service %s\n", SERVICE_NAME);
    }
   
    if (kr != KERN_SUCCESS) {
        // NB: When unregistering, we will get here if the unregister succeeded.
        mach_error("bootstrap_register", kr);
        exit(kr);
    }
   
    MY_ASL_LOG("server loop ready\n");
   
    while (1) // Dummy server loop.
        sleep(60);
   
    exit(0);
}
