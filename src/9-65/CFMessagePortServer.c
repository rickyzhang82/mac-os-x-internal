// CFMessagePortServer.c
   
#include <CoreFoundation/CoreFoundation.h>
   
#define LOCAL_NAME "com.osxbook.CFMessagePort.server"
   
CFDataRef
localPortCallBack(CFMessagePortRef local, SInt32 msgid, CFDataRef data,
                  void *info)
{
    printf("message received\n");
    CFShow(data);
    return NULL;
}
   
int
main(void)
{
    CFMessagePortRef   localPort;
    CFRunLoopSourceRef runLoopSource;
   
    localPort = CFMessagePortCreateLocal(
                    kCFAllocatorDefault, // allocator
                    CFSTR(LOCAL_NAME),   // name for registering the port
                    localPortCallBack,   // call this when message received
                    NULL,                // contextual information
                    NULL);               // free "info" field of context?
    if (localPort == NULL) {
        fprintf(stderr, "*** CFMessagePortCreateLocal\n");
        exit(1);
    }
   
    runLoopSource = CFMessagePortCreateRunLoopSource(
                        kCFAllocatorDefault, // allocator
                        localPort, // create run-loop source for this port
                        0);        // priority index
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,
                       kCFRunLoopCommonModes);
    CFRunLoopRun();
   
    CFRelease(runLoopSource);
    CFRelease(localPort);
   
    exit(0);
}
