// kq_fwatch.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/event.h>
#include <unistd.h>
   
#define PROGNAME "kq_fwatch"
   
typedef struct {
    u_int       event;
    const char *description;
} VnEventDescriptions_t;
   
VnEventDescriptions_t VnEventDescriptions[] = {
    { NOTE_ATTRIB, "attributes changed"                      },
    { NOTE_DELETE, "deleted"                                 },
    { NOTE_EXTEND, "extended"                                },
    { NOTE_LINK,   "link count changed"                      },
    { NOTE_RENAME, "renamed"                                 },
    { NOTE_REVOKE, "access revoked or file system unmounted" },
    { NOTE_WRITE,  "written"                                 },
};
#define N_EVENTS (sizeof(VnEventDescriptions)/sizeof(VnEventDescriptions_t))
   
int
process_events(struct kevent *kl)
{
    int i, ret = 0;
   
    for (i = 0; i < N_EVENTS; i++)
        if (VnEventDescriptions[i].event & kl->fflags)
            printf("%s\n", VnEventDescriptions[i].description);
   
    if (kl->fflags & NOTE_DELETE) // stop when the file is gone
        ret = -1;
   
    return ret;
}
   
int
main(int argc, char **argv)
{
    int fd, ret = -1, kqfd = -1;
    struct kevent changelist;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file to watch>\n", PROGNAME);
        exit(1);
    }
   
    // create a new kernel event queue (not inherited across fork())
    if ((kqfd = kqueue()) < 0) {
        perror("kqueue");
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
   
#define NOTE_ALL NOTE_ATTRIB |\
                 NOTE_DELETE |\
                 NOTE_EXTEND |\
                 NOTE_LINK   |\
                 NOTE_RENAME |\
                 NOTE_REVOKE |\
                 NOTE_WRITE
    EV_SET(&changelist, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_ALL, 0, NULL);
    // the following kevent() call is for registering events
    ret = kevent(kqfd,        // kqueue file descriptor
                 &changelist, // array of kevent structures
                 1,           // number of entries in the changelist array
                 NULL,        // array of kevent structures (for receiving)
                 0,           // number of entries in the above array
                 NULL);       // timeout
    if (ret < 0) {
        perror("kqueue");
        goto out;
    }
           
    do {
        // the following kevent() call is for receiving events
        // we recycle the changelist from the previous call
        if ((ret = kevent(kqfd, NULL, 0, &changelist, 1, NULL)) == -1) {
            perror("kevent");
            goto out; 
        }
   
        // kevent() returns the number of events placed in the receive list
        if (ret != 0)
            ret = process_events(&changelist);
   
    } while (!ret);
    
out:
    if (kqfd >= 0) 
        close(kqfd);
   
    exit(ret);
}
