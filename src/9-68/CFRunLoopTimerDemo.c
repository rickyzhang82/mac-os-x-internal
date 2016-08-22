// CFRunLoopTimerDemo.c
   
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
   
void timerCallBack(CFRunLoopTimerRef timer, void *info);
   
void
timerCallBack(CFRunLoopTimerRef timer, void *info)
{
    CFShow(timer);
}
   
int
main(int argc, char **argv)
{
    CFRunLoopTimerRef runLoopTimer = CFRunLoopTimerCreate(
        kCFAllocatorDefault,              // allocator
        CFAbsoluteTimeGetCurrent() + 2.0, // fire date (now + 2 seconds)
        1.0,           // fire interval (0 or -ve means a one-shot timer)
        0,             // flags (ignored)
        0,             // order (ignored)
        timerCallBack, // called when the timer fires
        NULL);         // context
   
    CFRunLoopAddTimer(CFRunLoopGetCurrent(),  // the run loop to use
                      runLoopTimer,           // the run-loop timer to add
                      kCFRunLoopDefaultMode); // add timer to this mode
   
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, // run it in this mode
                       4.0,    // run it for this long
                       false); // exit after processing one source?
   
    printf("Run Loop stopped\n");
   
    // sleep for a bit to show that the timer is not processed any more
    sleep(4);
   
    CFRunLoopTimerInvalidate(runLoopTimer);
    CFRelease(runLoopTimer);
   
    exit(0);
}
