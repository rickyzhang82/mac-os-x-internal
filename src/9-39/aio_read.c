// aio_read.c
   
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <aio.h>
   
#define PROGNAME "aio_read"
   
#define AIO_BUFSIZE 4096
#define AIOCB_CONST struct aiocb *const*
   
static void
SIGUSR1_handler(int signo __unused)
{
    printf("SIGUSR1_handler\n");
}
   
int
main(int argc, char **argv)
{
    int               fd;
    struct aiocb     *aiocbs[1], aiocb;
    struct sigaction  act;
    char              buf[AIO_BUFSIZE];
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file path>\n", PROGNAME);
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
   
    aiocbs[0] = &aiocb;
   
    aiocb.aio_fildes = fd;
    aiocb.aio_offset = (off_t)0;
    aiocb.aio_buf    = buf;
    aiocb.aio_nbytes = AIO_BUFSIZE;
   
    // not used on Mac OS X
    aiocb.aio_reqprio = 0;
   
    // we want to be notified via a signal when the asynchronous I/O finishes
    // SIGEV_THREAD (notification via callback) is not supported on Mac OS X
    aiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
   
    // send this signal when done: must be valid (except SIGKILL or SIGSTOP)
    aiocb.aio_sigevent.sigev_signo = SIGUSR1;
   
    // ignored on Mac OS X
    aiocb.aio_sigevent.sigev_value.sival_int = 0;
    aiocb.aio_sigevent.sigev_notify_function = (void(*)(union sigval))0;
    aiocb.aio_sigevent.sigev_notify_attributes = (pthread_attr_t *)0;
   
    aiocb.aio_lio_opcode = LIO_READ;
   
    // set up a handler for SIGUSR1
    act.sa_handler = SIGUSR1_handler;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
   
    // initiates a list of I/O requests specified by a list of aiocb structures
    if (lio_listio(LIO_NOWAIT, (AIOCB_CONST)aiocbs, 1, &(aiocb.aio_sigevent)))
        perror("lio_listio");
    else {
        printf("asynchronous read issued...\n");
   
        // quite contrived, since we could have used LIO_WAIT with lio_listio()
        // anyway, the I/O might already be done by the time we call this
        aio_suspend((const AIOCB_CONST)aiocbs, 1, (const struct timespec *)0);
    }
   
    return 0;
}
