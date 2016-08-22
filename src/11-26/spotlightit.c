// spotlightit.c
   
#include <Carbon/Carbon.h>
   
#define PROGNAME "spotlightit"
   
int
main(int argc, char **argv)
{
    OSStatus status;
    CFStringRef searchString;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <search string>\n", PROGNAME);
        return 1;
    }
   
    searchString = CFStringCreateWithCString(kCFAllocatorDefault, argv[1],
                                             kCFStringEncodingUTF8);
    status = HISearchWindowShow(searchString, kNilOptions);
    CFRelease(searchString);
                     
    return (int)status;
}
