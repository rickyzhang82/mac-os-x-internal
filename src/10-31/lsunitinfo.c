// lsunitinfo.c
   
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
   
int
main(void)
{
    kern_return_t kr;
    io_service_t  pexpert;
    CFStringRef   serial, model;
   
    // get the Platform Expert object
    pexpert = IOServiceGetMatchingService(kIOMasterPortDefault,
                  IOServiceMatching("IOPlatformExpertDevice"));
    if (!pexpert)
        return KERN_FAILURE;
   
    serial = IORegistryEntryCreateCFProperty(
                 pexpert, CFSTR(kIOPlatformSerialNumberKey),
                 kCFAllocatorDefault,kNilOptions);
    if (serial) {
        // note that this will go to stderr
        CFShow(serial);
        CFRelease(serial);
    }
   
    model = IORegistryEntryCreateCFProperty(
                pexpert, CFSTR("model"), kCFAllocatorDefault, kNilOptions);
    if (model) {
        printf("%s\n", CFDataGetBytePtr((CFDataRef)model));
        CFRelease(model);
    }
   
    if (pexpert)
        IOObjectRelease(pexpert);
   
    return kr;
}
