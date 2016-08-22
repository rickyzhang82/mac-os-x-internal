// CFNotificationPoster.c
   
#include <CoreFoundation/CoreFoundation.h>
   
#define PROGNAME "cfposter"
   
int
main(int argc, char **argv)
{
    CFStringRef             name, object;
    CFNotificationCenterRef distributedCenter;
    CFStringEncoding        encoding = kCFStringEncodingASCII;
   
    if (argc != 3) {
        fprintf(stderr, "usage: %s <name string> <value string>\n", PROGNAME);
        exit(1);
    }
   
    name = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], encoding);
    object = CFStringCreateWithCString(kCFAllocatorDefault, argv[2], encoding);
   
    distributedCenter = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(
        distributedCenter, // the notification center to use
        name,              // name of the notification to post
        object,            // optional object identifier
        NULL,              // optional dictionary of "user" information
        false);            // deliver immediately (if true) or respect the
                           // suspension behaviors of observers (if false)
   
CFRelease(distributedCenter);
    CFRelease(name);
    CFRelease(object);
   
    exit(0);
}
