// kq_vfswatch.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/mount.h>
#include <unistd.h>
   
#define PROGNAME "kq_vfswatch"
   
struct VfsEventDescriptions{
    u_int       event;
    const char *description;
} VfsEventDescriptions[] = {
     { VQ_NOTRESP,      "server is down"                                     },
     { VQ_NEEDAUTH,     "server needs authentication"                        },
     { VQ_LOWDISK,      "disk space is low"                                  },
     { VQ_MOUNT,        "file system mounted"                                },
     { VQ_UNMOUNT,      "file system unmounted"                              },
     { VQ_DEAD,         "file system is dead (needs force unmount)"          },
     { VQ_ASSIST,       "file system needs assistance from external program" },
     { VQ_NOTRESPLOCK,  "server locked down"                                 },
     { VQ_UPDATE,       "file system information has changed"                },
};
#define NEVENTS sizeof(VfsEventDescriptions)/sizeof(struct VfsEventDescriptions)
   
int
process_events(struct kevent *kl)
{
    int i, ret = 0;
   
    printf("notification received\n");
    for (i = 0; i < NEVENTS; i++)
        if (VfsEventDescriptions[i].event & kl->fflags)
            printf("\t+ %s\n", VfsEventDescriptions[i].description);
   
    return ret;
}
   
#define OUT_ON_ERROR(msg, ret) { if (ret < 0) { perror(msg); goto out; } }
   
int
main(int argc, char **argv)
{
    int ret = -1, kqfd = -1;
    struct kevent changelist;
   
    ret = kqfd = kqueue();
    OUT_ON_ERROR("kqueue", ret);
   
    EV_SET(&changelist, 0, EVFILT_FS, EV_ADD, 0, 0, NULL);
    ret = kevent(kqfd, &changelist, 1, NULL, 0, NULL);
    OUT_ON_ERROR("kqueue", ret);
           
    while (1) {
        ret = kevent(kqfd, NULL, 0, &changelist, 1, NULL);
        OUT_ON_ERROR("kevent", ret);
   
        if (ret > 0)
            ret = process_events(&changelist);
    }
    
out:
    if (kqfd >= 0) 
        close(kqfd);
   
    exit(ret);
}
