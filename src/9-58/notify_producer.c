// notify_producer.c
   
#include "notify_common.h"
   
#define PROGNAME "notify_producer"
   
int
usage(void)
{
    fprintf(stderr, "usage: %s -c|-f|-p|-s\n", PROGNAME);
    return 1;
}
   
int
main(int argc, char **argv)
{
    int   ch, options = 0;
    char *name;
   
    if (argc != 2)
        return usage();
   
    while ((ch = getopt(argc, argv, "cfps")) != -1) {
        switch (ch) {
        case 'c':
            name = NOTIFICATION_CANCEL;
            break;
        case 'f':
            name = NOTIFICATION_BY_FILE_DESCRIPTOR;
            break;
        case 'p':
            name = NOTIFICATION_BY_MACH_PORT;
            break;
        case 's':
            name = NOTIFICATION_BY_SIGNAL;
            break;
        default:
            return usage();
            break;
        }
        options++;
    }
   
    if (options == 1)
        return (int)notify_post(name);
    else
        return usage();
}
