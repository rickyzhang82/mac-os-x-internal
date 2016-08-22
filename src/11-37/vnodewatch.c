// vnodewatch.c
   
#include <IOKit/IOKitLib.h>
#include <IOKit/IODataQueueShared.h>
#include <IOKit/IODataQueueClient.h>
   
#include <mach/mach.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/acl.h>
   
#include "VnodeWatcher.h"
   
#define PROGNAME "vnodewatch"
#define VNODE_WATCHER_IOKIT_CLASS "com_osxbook_driver_VnodeWatcher"
   
#define printIfAction(action, name) \
    { if (action & KAUTH_VNODE_##name) { printf("%s ", #name); } }
   
void
action_print(UInt32 action, int isdir)
{
    printf("{ ");
   
    if (isdir)
        goto dir;
   
    printIfAction(action, READ_DATA);   // read contents of file
    printIfAction(action, WRITE_DATA);  // write contents of file
    printIfAction(action, EXECUTE);     // execute contents of file
    printIfAction(action, APPEND_DATA); // append to contents of file
    goto common;
   
dir:
    printIfAction(action, LIST_DIRECTORY);   // enumerate directory contents
    printIfAction(action, ADD_FILE);         // add file to directory
    printIfAction(action, SEARCH);           // look up specific directory item
    printIfAction(action, ADD_SUBDIRECTORY); // add subdirectory in directory
    printIfAction(action, DELETE_CHILD);     // delete an item in directory
   
common:
    printIfAction(action, DELETE);              // delete a file system object
    printIfAction(action, READ_ATTRIBUTES);     // read standard attributes
    printIfAction(action, WRITE_ATTRIBUTES);    // write standard attributes
    printIfAction(action, READ_EXTATTRIBUTES);  // read extended attributes
    printIfAction(action, WRITE_EXTATTRIBUTES); // write extended attributes
    printIfAction(action, READ_SECURITY);       // read ACL
    printIfAction(action, WRITE_SECURITY);      // write ACL
    printIfAction(action, TAKE_OWNERSHIP);      // change ownership
    // printIfAction(action, SYNCHRONIZE);      // unused
    printIfAction(action, LINKTARGET);          // create a new hard link
    printIfAction(action, CHECKIMMUTABLE);      // check for immutability
   
    printIfAction(action, ACCESS);              // special flag
    printIfAction(action, NOIMMUTABLE);         // special flag
   
    printf("}\n");
}
   
const char *
vtype_name(enum vtype vtype)
{
    static const char *vtype_names[] = {
        "VNON",  "VREG",  "VDIR", "VBLK", "VCHR", "VLNK",
        "VSOCK", "VFIFO", "VBAD", "VSTR", "VCPLX",
    };
   
    return vtype_names[vtype];
}
   
const char *
vtag_name(enum vtagtype vtag)
{
    static const char *vtag_names[] = {
        "VT_NON",   "VT_UFS",    "VT_NFS",    "VT_MFS",    "VT_MSDOSFS",
        "VT_LFS",   "VT_LOFS",   "VT_FDESC",  "VT_PORTAL", "VT_NULL",
        "VT_UMAP",  "VT_KERNFS", "VT_PROCFS", "VT_AFS",    "VT_ISOFS",
        "VT_UNION", "VT_HFS",    "VT_VOLFS",  "VT_DEVFS",  "VT_WEBDAV",
        "VT_UDF",   "VT_AFP",    "VT_CDDA",   "VT_CIFS",   "VT_OTHER",
    };
   
    return vtag_names[vtag];
}
   
static IOReturn
vnodeNotificationHandler(io_connect_t connection)
{
    kern_return_t       kr; 
    VnodeWatcherData_t  vdata;
    UInt32              dataSize;
    IODataQueueMemory  *queueMappedMemory;
    vm_size_t           queueMappedMemorySize;
    vm_address_t        address = nil;
    vm_size_t           size = 0;
    unsigned int        msgType = 1; // family-defined port type (arbitrary)
    mach_port_t         recvPort;
        
    // allocate a Mach port to receive notifications from the IODataQueue
    if (!(recvPort = IODataQueueAllocateNotificationPort())) {
        fprintf(stderr, "%s: failed to allocate notification port\n", PROGNAME);
        return kIOReturnError;
    }
    
    // this will call registerNotificationPort() inside our user client class
    kr = IOConnectSetNotificationPort(connection, msgType, recvPort, 0);
    if (kr != kIOReturnSuccess) {
        fprintf(stderr, "%s: failed to register notification port (%d)\n",
                PROGNAME, kr);
        mach_port_destroy(mach_task_self(), recvPort);
        return kr;
    }
    
    // this will call clientMemoryForType() inside our user client class
    kr = IOConnectMapMemory(connection, kIODefaultMemoryType,
                            mach_task_self(), &address, &size, kIOMapAnywhere);
    if (kr != kIOReturnSuccess) {
        fprintf(stderr, "%s: failed to map memory (%d)\n", PROGNAME, kr);
        mach_port_destroy(mach_task_self(), recvPort);
        return kr;
    }
    
    queueMappedMemory = (IODataQueueMemory *)address;
    queueMappedMemorySize = size;    
    
    while (IODataQueueWaitForAvailableData(queueMappedMemory, recvPort) ==
           kIOReturnSuccess) {            
        while (IODataQueueDataAvailable(queueMappedMemory)) {   
            dataSize = sizeof(vdata);
            kr = IODataQueueDequeue(queueMappedMemory, &vdata, &dataSize);
            if (kr == kIOReturnSuccess) {
   
                if (*(UInt8 *)&vdata == kt_kStopListeningToMessages)
                    goto exit;
            
                printf("\"%s\" %s %s %lu(%s) ",
                       vdata.path,
                       vtype_name(vdata.v_type),
                       vtag_name(vdata.v_tag),
                       vdata.pid,
                       vdata.p_comm);
                action_print(vdata.action, (vdata.v_type & VDIR));
            } else
                fprintf(stderr, "*** error in receiving data (%d)\n", kr);
        }
    }
   
exit:
    
    kr = IOConnectUnmapMemory(connection, kIODefaultMemoryType,
                              mach_task_self(), address);
    if (kr != kIOReturnSuccess)
        fprintf(stderr, "%s: failed to unmap memory (%d)\n", PROGNAME, kr);
    
    mach_port_destroy(mach_task_self(), recvPort);
    
    return kr;
}
   
#define PRINT_ERROR_AND_RETURN(msg, ret) \
    { fprintf(stderr, "%s: %s\n", PROGNAME, msg); return ret; }
   
int
main(int argc, char **argv)
{
    kern_return_t   kr; 
    int             ret;
    io_iterator_t   iterator;
    io_service_t    serviceObject;
    CFDictionaryRef classToMatch;
    pthread_t       dataQueueThread;
    io_connect_t    connection;
    
    setbuf(stdout, NULL);
   
    if (!(classToMatch = IOServiceMatching(VNODE_WATCHER_IOKIT_CLASS)))
        PRINT_ERROR_AND_RETURN("failed to create matching dictionary", -1);
    
    kr = IOServiceGetMatchingServices(kIOMasterPortDefault, classToMatch,
                                      &iterator);
    if (kr != kIOReturnSuccess)
        PRINT_ERROR_AND_RETURN("failed to retrieve matching services", -1);
    
    serviceObject = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (!serviceObject)
        PRINT_ERROR_AND_RETURN("VnodeWatcher service not found", -1);
    
    kr = IOServiceOpen(serviceObject, mach_task_self(), 0, &connection);
    IOObjectRelease(serviceObject);
    if (kr != kIOReturnSuccess)
        PRINT_ERROR_AND_RETURN("failed to open VnodeWatcher service", kr);
   
    kr = IOConnectMethodScalarIScalarO(connection,
                                       kt_kVnodeWatcherUserClientOpen, 0, 0);
    if (kr != KERN_SUCCESS) {
        (void)IOServiceClose(connection);
        PRINT_ERROR_AND_RETURN("VnodeWatcher service is busy", kr);
    }
    
    ret = pthread_create(&dataQueueThread, (pthread_attr_t *)0,
                         (void *)vnodeNotificationHandler, (void *)connection);
    if (ret)
        perror("pthread_create");
    else
        pthread_join(dataQueueThread, (void **)&kr);
   
    (void)IOServiceClose(connection);
                  
    return 0;
}
