// VnodeWatcher.h
   
#include <sys/param.h>
#include <sys/kauth.h>
#include <sys/vnode.h>
   
typedef struct {
    UInt32        pid;
    UInt32        action;
    enum vtype    v_type;
    enum vtagtype v_tag;;
    char          p_comm[MAXCOMLEN + 1];
    char          path[MAXPATHLEN];
} VnodeWatcherData_t;
   
enum {
    kt_kVnodeWatcherUserClientOpen,
    kt_kVnodeWatcherUserClientClose,
    kt_kVnodeWatcherUserClientNMethods,
    kt_kStopListeningToMessages = 0xff,
};
   
#define VNW_LOG_FILE "/private/tmp/VnodeWatcher.log"
   
#ifdef KERNEL
   
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IODataQueue.h>
#include <sys/types.h>
#include <sys/kauth.h>
   
// the I/O Kit driver class
class com_osxbook_driver_VnodeWatcher : public IOService
{
    OSDeclareDefaultStructors(com_osxbook_driver_VnodeWatcher)
   
public:
    virtual bool start(IOService *provider);
};
   
enum { kt_kMaximumEventsToHold = 512 };
   
// the user client class
class com_osxbook_driver_VnodeWatcherUserClient : public IOUserClient
{
    OSDeclareDefaultStructors(com_osxbook_driver_VnodeWatcherUserClient)
    
private:
    task_t                           fClient;
    com_osxbook_driver_VnodeWatcher *fProvider;
    IODataQueue                     *fDataQueue;
    IOMemoryDescriptor              *fSharedMemory;
    kauth_listener_t                 fListener;
   
public:
    virtual bool     start(IOService *provider);
    virtual void     stop(IOService *provider);
    virtual IOReturn open(void);
    virtual IOReturn clientClose(void);
    virtual IOReturn close(void);
    virtual bool     terminate(IOOptionBits options);
    virtual IOReturn startLogging(void);
    virtual IOReturn stopLogging(void);
   
    virtual bool     initWithTask(
                         task_t owningTask, void *securityID, UInt32 type);
    virtual IOReturn registerNotificationPort(
                         mach_port_t port, UInt32 type, UInt32 refCon);
   
    virtual IOReturn clientMemoryForType(UInt32 type, IOOptionBits *options,
                                         IOMemoryDescriptor **memory);
    virtual IOExternalMethod *getTargetAndMethodForIndex(IOService **target,
                                                         UInt32 index);
};
   
#endif // KERNEL
