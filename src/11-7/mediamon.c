// mediamon.c
   
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <CoreFoundation/CoreFoundation.h>
   
int
printDictionaryAsXML(CFDictionaryRef dict)
{
    CFDataRef xml = CFPropertyListCreateXMLData(kCFAllocatorDefault,
                                                (CFPropertyListRef)dict);
    if (!xml)
        return -1;
   
    write(STDOUT_FILENO, CFDataGetBytePtr(xml), CFDataGetLength(xml));
    CFRelease(xml);
   
    return 0;
}
   
void
matchingCallback(void *refcon, io_iterator_t deviceList)
{
    kern_return_t       kr;
    CFDictionaryRef     properties;
    io_registry_entry_t device;
   
    // Iterate over each device in this notification.
    while ((device = IOIteratorNext(deviceList))) {
   
        // Populate a dictionary with device's properties.
        kr = IORegistryEntryCreateCFProperties(
                 device, (CFMutableDictionaryRef *)&properties,
                 kCFAllocatorDefault, kNilOptions);
   
        if (kr == KERN_SUCCESS)
            printDictionaryAsXML(properties);
   
        if (properties)
            CFRelease(properties);
   
        if (device)
            IOObjectRelease(device);
    }
}
   
int
main(void)
{
    CFMutableDictionaryRef match;
    IONotificationPortRef  notifyPort;
    CFRunLoopSourceRef     notificationRunLoopSource;
    io_iterator_t          notificationIn, notificationOut;
   
    // Create a matching dictionary for all IOMedia objects.
    if (!(match = IOServiceMatching("IOMedia"))) {
        fprintf(stderr, "*** failed to create matching dictionary.\n");
        exit(1);
    }
   
    // Create a notification object for receiving I/O Kit notifications.
    notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
   
    // Get a CFRunLoopSource that we will use to listen for notifications.
    notificationRunLoopSource = IONotificationPortGetRunLoopSource(notifyPort);
   
    // Add the CFRunLoopSource to the default mode of our current run loop.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), notificationRunLoopSource,
                       kCFRunLoopDefaultMode);
   
    // One reference of the matching dictionary will be consumed when we install
    // a notification request. Since we need to install two such requests (one
    // for ejectable media coming in and another for it going out), we need
    // to increment the reference count on our matching dictionary.
    CFRetain(match);
   
    // Install notification request for matching objects coming in.
    // Note that this will also look up already existing objects.
    IOServiceAddMatchingNotification(
        notifyPort,             // notification port reference
        kIOMatchedNotification, // notification type
        match,                  // matching dictionary
        matchingCallback,       // this is called when notification fires
        NULL,                   // reference constant
        &notificationIn);       // iterator handle
   
    // Install notification request for matching objects going out.
    IOServiceAddMatchingNotification(
        notifyPort,
        kIOTerminatedNotification,
        match,
        matchingCallback,
        NULL,
        &notificationOut);
   
    // Invoke callbacks explicitly to empty the iterators/arm the notifications.
    matchingCallback(0, notificationIn);
    matchingCallback(0, notificationOut);
   
    CFRunLoopRun(); // run
   
    exit(0);
}
