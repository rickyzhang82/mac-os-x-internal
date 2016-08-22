// diskarb_info.c
   
#include <unistd.h>
#include <DiskArbitration/DiskArbitration.h>
   
#define DEFAULT_DISK_NAME "/dev/disk0"
   
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
   
#define OUT_ON_NULL(ptr, msg) \
    if (!ptr) { fprintf(stderr, "%s\n", msg); goto out; }
   
int
main(int argc, char **argv)
{
    int              ret      = -1;
    DASessionRef     session  = NULL;
    DADiskRef        disk     = NULL;
    CFDictionaryRef  diskInfo = NULL;
    char            *diskName = DEFAULT_DISK_NAME;
   
    // create a new Disk Arbitration session
    session = DASessionCreate(kCFAllocatorDefault);
    OUT_ON_NULL(session, "failed to create Disk Arbitration session");
   
    if (argc == 2)
        diskName = argv[1];
   
    // create a new disk object from the given BSD device name
    disk = DADiskCreateFromBSDName(kCFAllocatorDefault, session, diskName);
    OUT_ON_NULL(disk, "failed to create disk object");
   
    // obtain disk's description
    diskInfo = DADiskCopyDescription(disk);
    OUT_ON_NULL(diskInfo, "failed to retrieve disk description");
   
    ret = printDictionaryAsXML(diskInfo);
   
out:
    if (diskInfo)
        CFRelease(diskInfo);
    if (disk)
        CFRelease(disk);
    if (session)
        CFRelease(session);
   
    exit(ret);
}
