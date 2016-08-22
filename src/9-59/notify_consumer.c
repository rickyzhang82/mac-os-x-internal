// notify_consumer.c
   
#include "notify_common.h"
#include <pthread.h>
#include <mach/mach.h>
#include <signal.h>
   
void sighandler_USR1(int s);
void cancel_all_notifications(void);
static int token_fd = -1, token_mach_port = -1, token_signal = -1;
static int token_mach_port_cancel = -1;
   
void *
consumer_file_descriptor(void *arg)
{
    int     status;
    int fd, check;
   
    status = notify_register_file_descriptor(NOTIFICATION_BY_FILE_DESCRIPTOR,
                                             &fd, 0, &token_fd);
    if (status != NOTIFY_STATUS_OK) {
        perror("notify_register_file_descriptor");
        return (void *)status;
    }
   
    while (1) {
        if ((status = read(fd, &check, sizeof(check))) < 0)
            return (void *)status; // perhaps the notification was canceled
        if (check == token_fd)
            printf("file descriptor: received notification\n");
        else
            printf("file descriptor: spurious notification?\n");
    }
   
    return (void *)0;
}
   
void *
consumer_mach_port(void *arg)
{
    int               status;
    kern_return_t     kr;
    mach_msg_header_t msg;
    mach_port_t       notify_port;
   
    status = notify_register_mach_port(NOTIFICATION_BY_MACH_PORT, &notify_port,
                                       0, &token_mach_port);
    if (status != NOTIFY_STATUS_OK) {
        perror("notify_register_mach_port");
        return (void *)status;
    }
   
    // to support cancellation of all notifications and exiting, we register
    // a second notification here, but reuse the Mach port allocated above
    status = notify_register_mach_port(NOTIFICATION_CANCEL, &notify_port,
                                       NOTIFY_REUSE, &token_mach_port_cancel);
    if (status != NOTIFY_STATUS_OK) {
        perror("notify_register_mach_port");
        mach_port_deallocate(mach_task_self(), notify_port);
        return (void *)status;
    }
   
    while (1) {
        kr = mach_msg(&msg,                  // message buffer
                      MACH_RCV_MSG,          // option
                      0,                     // send size
                      MACH_MSG_SIZE_MAX,     // receive limit
                      notify_port,           // receive name
                      MACH_MSG_TIMEOUT_NONE, // timeout
                      MACH_PORT_NULL);       // cancel/receive notification
        if (kr != MACH_MSG_SUCCESS)
            mach_error("mach_msg(MACH_RCV_MSG)", kr);
   
        if (msg.msgh_id == token_mach_port)
            printf("Mach port: received notification\n");
        else if (msg.msgh_id == token_mach_port_cancel) {
            cancel_all_notifications();
            printf("canceling all notifications and exiting\n");
            exit(0);
        } else
            printf("Mach port: spurious notification?\n");
    }
   
    return (void *)0;
}
   
void
sighandler_USR1(int s)
{
    int status, check;
   
    status = notify_check(token_signal, &check);
    if ((status == NOTIFY_STATUS_OK) && (check != 0))
        printf("signal: received notification\n");
    else
        printf("signal: spurious signal?\n");
}
   
void *
consumer_signal(void *arg)
{
    int status, check;
   
    // set up signal handler
    signal(SIGUSR1, sighandler_USR1);
   
    status = notify_register_signal(NOTIFICATION_BY_SIGNAL, SIGUSR1,
                                    &token_signal);
    if (status != NOTIFY_STATUS_OK) {
        perror("notify_register_signal");
        return (void *)status;
    }
   
    // since notify_check() always sets check to 'true' when it is called for
    // the first time, we make a dummy call here
    (void)notify_check(token_signal, &check);
   
    while (1) {
        // just sleep for a day
        sleep(86400);
    }
 
    return (void *)0;
}
   
void
cancel_all_notifications(void)
{
    if (token_fd != -1)
        notify_cancel(token_fd);
    if (token_mach_port != -1)
        notify_cancel(token_mach_port);
    if (token_signal != -1)
        notify_cancel(token_signal);
}
   
int
main(int argc, char **argv)
{
    int ret;
    pthread_t pthread_fd, pthread_mach_port;
   
    if ((ret = pthread_create(&pthread_fd, (const pthread_attr_t *)0, 
                              consumer_file_descriptor, (void *)0)))
        goto out;
   
    if ((ret = pthread_create(&pthread_mach_port, (const pthread_attr_t *)0, 
                              consumer_mach_port, (void *)0)))
        goto out;
   
    if (consumer_signal((void *)0) != (void *)0)
        goto out;
   
out:
    cancel_all_notifications();
   
    return 0;
}
