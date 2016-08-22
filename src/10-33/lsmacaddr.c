// lsmacaddr.c
   
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <CoreFoundation/CoreFoundation.h>
   
typedef UInt8 MACAddress_t[kIOEthernetAddressSize];
   
void
printMACAddress(MACAddress_t MACAddress)
{
    int i;
   
    for (i = 0; i < kIOEthernetAddressSize - 1; i++)
        printf("%02x:", MACAddress[i]);
   
    printf("%x\n", MACAddress[i]);
}
   
int
main(void)
{
    kern_return_t          kr;
    CFMutableDictionaryRef classToMatch;
    io_iterator_t          ethernet_interfaces;
    io_object_t            ethernet_interface, ethernet_controller;
    CFTypeRef              MACAddressAsCFData;
   
    classToMatch = IOServiceMatching(kIOEthernetInterfaceClass);
    kr = IOServiceGetMatchingServices(kIOMasterPortDefault, classToMatch,
                                      &ethernet_interfaces);
    if (kr != KERN_SUCCESS)
        return kr;
   
    while ((ethernet_interface = IOIteratorNext(ethernet_interfaces))) {
   
        kr = IORegistryEntryGetParentEntry(ethernet_interface, kIOServicePlane,
                                           &ethernet_controller);
        if (kr != KERN_SUCCESS)
            goto next;
   
        MACAddressAsCFData = IORegistryEntryCreateCFProperty(
                                 ethernet_controller,
                                 CFSTR(kIOMACAddress),
                                 kCFAllocatorDefault, 0);
        if (MACAddressAsCFData) {
            MACAddress_t address;
            CFDataGetBytes(MACAddressAsCFData,
                           CFRangeMake(0, kIOEthernetAddressSize), address);
            CFRelease(MACAddressAsCFData);
            printMACAddress(address);
        }
        IOObjectRelease(ethernet_controller);
next:
        IOObjectRelease(ethernet_interface);
    }
    IOObjectRelease(ethernet_interfaces);
   
    return kr;
}
