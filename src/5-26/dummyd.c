// dummyd.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <launch.h>
#include <pthread.h>
   
#define MY_LAUNCH_JOBKEY_LISTENERS "Listeners"
   
// error-handling convenience
#define DO_RETURN(retval, fmt, ...) { \
    fprintf(stderr, fmt, ## __VA_ARGS__); \
    return retval; \
}
   
int
SIGTERM_handler(int s)
{
    fprintf(stderr, "SIGTERM handled\n"); // primitive SIGTERM handler
    exit(s);
}
   
ssize_t
readline(int fd, void *buffer, size_t maxlen)
{
    ssize_t n, bytesread;
    char c, *bp = buffer;
   
    for (n = 1; n < maxlen; n++) {
        bytesread = read(fd, &c, 1);
        if (bytesread == 1) {
            *bp++ = c;
            if (c == '\n')
                break;
        } else if (bytesread == 0) {
            if (n == 1)
                return 0;
            break;
        } else {
            if (errno == EINTR)
                continue;
            return -1;
        }
    }
   
    *bp = 0;
   
    return n;
}
   
void *
daemon_loop(void *fd)
{
    ssize_t ret;
    char    buf[512];
   
    for (;;) { // a simple echo loop
        if ((ret = readline((int)fd, buf, 512)) > 0)
            write((int)fd, buf, ret);
        else {
            close((int)fd);
            return (void *)0;
        }
    }
}
   
int
main(void)
{
    char           path[MAXPATHLEN + 1];
    char          *val;
    int            fd, kq;
    size_t         i;
    pthread_t      thread;
    struct kevent  kev_init, kev_listener;
    struct         sockaddr_storage ss;
    socklen_t      slen;
    launch_data_t  checkin_response, checkin_request;
    launch_data_t  sockets_dict, listening_fd_array;
   
    setbuf(stderr, NULL); // make stderr unbuffered
   
    // launchd will send us a SIGTERM while terminating
    signal(SIGTERM, (sig_t)SIGTERM_handler);
   
    // print our cwd: our configuration file specified this
    if (getcwd(path, MAXPATHLEN))
        fprintf(stderr, "Working directory: %s\n", path);
    
    // print $DUMMY_VARIABLE: our configuration file specified this
    fprintf(stderr, "Special enivronment variables: ");
    if ((val = getenv("DUMMY_VARIABLE")))
        fprintf(stderr, "DUMMY_VARIABLE=%s\n", val);
   
    if ((kq = kqueue()) == -1) // create a kernel event queue for notification
        DO_RETURN(EXIT_FAILURE, "kqueue() failed\n");
   
    // prepare to check in with launchd
    checkin_request = launch_data_new_string(LAUNCH_KEY_CHECKIN);
    if (checkin_request == NULL)
        DO_RETURN(EXIT_FAILURE, "launch_data_new_string(%s) failed (errno = %d)"
                  "\n", LAUNCH_KEY_CHECKIN, errno);
   
    checkin_response = launch_msg(checkin_request); // check in with launchd
    if (checkin_response == NULL)
        DO_RETURN(EXIT_FAILURE, "launch_msg(%s) failed (errno = %d)\n",
                  LAUNCH_KEY_CHECKIN, errno);
    if (launch_data_get_type(checkin_response) == LAUNCH_DATA_ERRNO)
        DO_RETURN(EXIT_FAILURE, "failed to check in with launchd (errno = %d)"
                  "\n", launch_data_get_errno(checkin_response));
   
    // retrieve the contents of the <Sockets> dictionary
    sockets_dict = launch_data_dict_lookup(checkin_response,
                                           LAUNCH_JOBKEY_SOCKETS);
    if (sockets_dict == NULL)
        DO_RETURN(EXIT_FAILURE, "no sockets\n");
   
    // retrieve the value of the MY_LAUNCH_JOBKEY_LISTENERS key
    listening_fd_array = launch_data_dict_lookup(sockets_dict,
                                                 MY_LAUNCH_JOBKEY_LISTENERS);
    if (listening_fd_array == NULL)
        DO_RETURN(EXIT_FAILURE, "no listening socket descriptors\n");
   
    for (i = 0; i < launch_data_array_get_count(listening_fd_array); i++) {
   
        launch_data_t fd_i = launch_data_array_get_index(listening_fd_array, i);
   
        EV_SET(&kev_init,                // the structure to populate
               launch_data_get_fd(fd_i), // identifier for this event
               EVFILT_READ,              // return on incoming connection
               EV_ADD,                   // flags: add the event to the kqueue
               0,                        // filter-specific flags (none)
               0,                        // filter-specific data (none)
               NULL);                    // opaque user-defined value (none)
        if (kevent(kq,                   // the kernel queue
                   &kev_init,            // changelist
                   1,                    // nchanges
                   NULL,                 // eventlist
                   0,                    // nevents
                   NULL) == -1)          // timeout
            DO_RETURN(EXIT_FAILURE, "kevent(/* register */) failed\n");
    }
   
    launch_data_free(checkin_response);
   
    while (1) {
   
        if ((fd = kevent(kq, NULL, 0, &kev_listener, 1, NULL)) == -1)
            DO_RETURN(EXIT_FAILURE, "kevent(/* get events */) failed\n");
   
        if (fd == 0)
            return EXIT_SUCCESS;
   
        slen = sizeof(ss);
        fd = accept(kev_listener.ident, (struct sockaddr *)&ss, &slen);
        if (fd == -1)
            continue;
   
        if (pthread_create(&thread, (pthread_attr_t *)0, daemon_loop,
                           (void *)fd) != 0) {
            close(fd);
            DO_RETURN(EXIT_FAILURE, "pthread_create() failed\n");
        }
   
        pthread_detach(thread);
    }
   
    return EXIT_SUCCESS;
}
