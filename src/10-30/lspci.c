// lspci.c
   
#include <stdio.h>
#include <IOKit/IOKitLib.h>
   
int
main(void)
{
    kern_return_t kr;
    io_iterator_t pciDeviceList;
    io_service_t  pciDevice;
    io_name_t     deviceName;
    io_string_t   devicePath;
   
    // get an iterator for all PCI devices
    if (IOServiceGetMatchingServices(kIOMasterPortDefault,
                                     IOServiceMatching("IOPCIDevice"),
                                     &pciDeviceList) != KERN_SUCCESS)
        return 1;
   
    while ((pciDevice = IOIteratorNext(pciDeviceList))) {
   
        kr = IORegistryEntryGetName(pciDevice, deviceName);
        if (kr != KERN_SUCCESS)
            goto next;
   
        kr = IORegistryEntryGetPath(pciDevice, kIOServicePlane, devicePath);
        if (kr != KERN_SUCCESS)
            goto next;
   
        // don't print the plane name prefix in the device path
        printf("%s (%s)\n", &devicePath[9], deviceName);
   
next:
        IOObjectRelease(pciDevice);
    }
   
    return kr;
}
