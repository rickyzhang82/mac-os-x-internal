// lsfirmware.c
   
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
   
#define PROGNAME "lsfirmware"
   
void
printDictionaryAsXML(CFDictionaryRef dict)
{
    CFDataRef xml = CFPropertyListCreateXMLData(kCFAllocatorDefault,
                                                (CFPropertyListRef)dict);
    if (xml) {
        write(STDOUT_FILENO, CFDataGetBytePtr(xml), CFDataGetLength(xml));
        CFRelease(xml);
    }
}
   
int
main(void)
{
    io_registry_entry_t    options;
    CFMutableDictionaryRef optionsDict;
    kern_return_t          kr = KERN_FAILURE;
   
    options = IORegistryEntryFromPath(kIOMasterPortDefault,
                                      kIODeviceTreePlane ":/options");
    if (options) {
        kr = IORegistryEntryCreateCFProperties(options, &optionsDict, 0, 0);
        if (kr == KERN_SUCCESS) {
            printDictionaryAsXML(optionsDict);
            CFRelease(optionsDict);
        }
        IOObjectRelease(options);
    }
   
    if (kr != KERN_SUCCESS)
        fprintf(stderr, "failed to retrieve firmware variables\n");
   
    exit(kr);
}
