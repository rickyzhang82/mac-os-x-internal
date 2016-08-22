// dissent_mount.c
   
#include <DiskArbitration/DiskArbitration.h>
   
#define OUT_ON_NULL(ptr, msg) \
    if (!ptr) { fprintf(stderr, "%s\n", msg); goto out; }
   
DADissenterRef
mountApprovalCallback(DADiskRef disk, void *context)
{
    DADissenterRef dissenter = DADissenterCreate(kCFAllocatorDefault,
                                                 kDAReturnNotPermitted,
                                                 CFSTR("mount disallowed"));
    printf("%s: mount disallowed\n", DADiskGetBSDName(disk));
    return dissenter;
}
   
int
main(void)
{
    DAApprovalSessionRef session = DAApprovalSessionCreate(kCFAllocatorDefault);
    OUT_ON_NULL(session, "failed to create Disk Arbitration session");
   
    DARegisterDiskMountApprovalCallback(session,
                                        NULL,  // matches all disk objects
                                        mountApprovalCallback,
                                        NULL); // context
   
    DAApprovalSessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(),
                                         kCFRunLoopDefaultMode);
   
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 30 /* seconds */, false);
   
    DAApprovalSessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(),
                                         kCFRunLoopDefaultMode);
   
    DAUnregisterApprovalCallback(session, mountApprovalCallback, NULL);
   
out:
   if (session)
        CFRelease(session);
   
    exit(0);
}
