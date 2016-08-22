// CFNotificationObserver.c
   
#include <CoreFoundation/CoreFoundation.h>
   
void
genericCallback(CFNotificationCenterRef  center,
                void                    *observer,
                CFStringRef              name,
                const void              *object,
                CFDictionaryRef          userInfo)
{
    if (!CFStringCompare(name, CFSTR("cancel"), kCFCompareCaseInsensitive)) {
        CFNotificationCenterRemoveObserver(center, observer, NULL, NULL);
        CFRunLoopStop(CFRunLoopGetCurrent());
    }
   
    printf("Received notification ==>\n");
    CFShow(center), CFShow(name), CFShow(object), CFShow(userInfo);
}
   
int
main(void)
{
    CFNotificationCenterRef distributedCenter;
    CFStringRef             observer = CFSTR("A CF Observer");
   
    distributedCenter = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterAddObserver(
        distributedCenter, // the notification center to use
        observer,          // an arbitrary observer-identifier
        genericCallback,   // callback to call when a notification is posted
        NULL,              // optional notification name to filter notifications
        NULL,              // optional object identifier to filter notifications
        CFNotificationSuspensionBehaviorDrop); // suspension behavior
   
    CFRunLoopRun();
   
    // not reached
    exit(0);
}
