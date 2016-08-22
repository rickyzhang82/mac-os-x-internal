// VnodeWatcher.cpp
   
#include <IOKit/IOLib.h>
#include <IOKit/IODataQueueShared.h>
#include <sys/proc.h>
#include "VnodeWatcher.h"
   
#define super IOService
OSDefineMetaClassAndStructors(com_osxbook_driver_VnodeWatcher, IOService)
   
static char   *gLogFilePath = NULL;
static size_t  gLogFilePathLen = 0;
static SInt32  gListenerInvocations = 0;
   
bool
com_osxbook_driver_VnodeWatcher::start(IOService *provider)
{
    if (!super::start(provider))
        return false;
        
    gLogFilePath = VNW_LOG_FILE;
    gLogFilePathLen = strlen(gLogFilePath) + 1;
   
    registerService();
   
    return true;
}
   
#undef super
#define super IOUserClient
OSDefineMetaClassAndStructors(
    com_osxbook_driver_VnodeWatcherUserClient, IOUserClient)
   
static const IOExternalMethod sMethods[kt_kVnodeWatcherUserClientNMethods] =
{
    {
        NULL,
        (IOMethod)&com_osxbook_driver_VnodeWatcherUserClient::open,
        kIOUCScalarIScalarO,
        0,
        0
    },
    {
        NULL,
        (IOMethod)&com_osxbook_driver_VnodeWatcherUserClient::close,
        kIOUCScalarIScalarO,
        0,
        0
    },
};
   
static int
my_vnode_authorize_callback(
    kauth_cred_t    credential, // reference to the actor's credentials
    void           *idata,      // cookie supplied when listener is registered
    kauth_action_t  action,     // requested action
    uintptr_t       arg0,       // the VFS context
    uintptr_t       arg1,       // the vnode in question
    uintptr_t       arg2,       // parent vnode, or NULL
    uintptr_t       arg3)       // pointer to an errno value
{
    UInt32 size;
    VnodeWatcherData_t data;
    int name_len = MAXPATHLEN;
   
    (void)OSIncrementAtomic(&gListenerInvocations); // enter the listener
   
    data.pid = vfs_context_pid((vfs_context_t)arg0);
    proc_name(data.pid, data.p_comm, MAXCOMLEN + 1);
    data.action = action;
    data.v_type = vnode_vtype((vnode_t)arg1);
    data.v_tag = (enum vtagtype)vnode_tag((vnode_t)arg1);
   
    size = sizeof(data) - sizeof(data.path);
   
    if (vn_getpath((vnode_t)arg1, data.path, &name_len) == 0)
        size += name_len;
    else {
        data.path[0] = '\0';
        size += 1;
    }
   
    if ((name_len != gLogFilePathLen) ||
        memcmp(data.path, gLogFilePath, gLogFilePathLen)) { // skip log file
        IODataQueue *q = OSDynamicCast(IODataQueue, (OSObject *)idata);
        q->enqueue(&data, size);
    }
   
    (void)OSDecrementAtomic(&gListenerInvocations); // leave the listener
   
    return KAUTH_RESULT_DEFER; // defer decision to other listeners
}
   
#define c_o_d_VUC com_osxbook_driver_VnodeWatcherUserClient
   
bool 
c_o_d_VUC::start(IOService *provider)
{
    fProvider = OSDynamicCast(com_osxbook_driver_VnodeWatcher, provider);
    if (!fProvider)
        return false;
   
    if (!super::start(provider))
        return false;
    
    fDataQueue = IODataQueue::withCapacity(
                     (sizeof(VnodeWatcherData_t)) * kt_kMaximumEventsToHold +
                     DATA_QUEUE_ENTRY_HEADER_SIZE);
    
    if (!fDataQueue)
        return kIOReturnNoMemory;
        
    fSharedMemory = fDataQueue->getMemoryDescriptor();
    if (!fSharedMemory) {
        fDataQueue->release();
        fDataQueue = NULL;
        return kIOReturnVMError;
    }    
    
    return true;
}
   
void 
c_o_d_VUC::stop(IOService *provider)
{
    if (fDataQueue) {
        UInt8 message = kt_kStopListeningToMessages;
        fDataQueue->enqueue(&message, sizeof(message));
    }
    
    if (fSharedMemory) {
        fSharedMemory->release();
        fSharedMemory = NULL;
    }
   
    if (fDataQueue) {
        fDataQueue->release();
        fDataQueue = NULL;
    }
   
    super::stop(provider);
}
   
IOReturn 
c_o_d_VUC::open(void)
{
    if (isInactive())
        return kIOReturnNotAttached;
        
    if (!fProvider->open(this))
        return kIOReturnExclusiveAccess; // only one user client allowed
   
    return startLogging();
}
   
IOReturn 
c_o_d_VUC::clientClose(void)
{
    (void)close();
    (void)terminate(0);
   
    fClient = NULL;
    fProvider = NULL;    
   
    return kIOReturnSuccess;
}
   
IOReturn 
c_o_d_VUC::close(void)
{
    if (!fProvider)
        return kIOReturnNotAttached;
        
    if (fProvider->isOpen(this))
        fProvider->close(this);
    
    return kIOReturnSuccess;
}
   
bool
c_o_d_VUC::terminate(IOOptionBits options)
{
    // if somebody does a kextunload while a client is attached
    if (fProvider && fProvider->isOpen(this))
        fProvider->close(this);
   
    (void)stopLogging();
   
    return super::terminate(options);
}
   
IOReturn
c_o_d_VUC::startLogging(void)
{
    
    fListener = kauth_listen_scope(              // register our listener
                    KAUTH_SCOPE_VNODE,           // for the vnode scope
                    my_vnode_authorize_callback, // using this callback
                    (void *)fDataQueue);         // give this cookie to callback
   
    if (fListener == NULL)
        return kIOReturnInternalError;
   
    return kIOReturnSuccess;
}
   
IOReturn
c_o_d_VUC::stopLogging(void)
{
    if (fListener != NULL) {
        kauth_unlisten_scope(fListener); // unregister our listener
        fListener = NULL;
    }
   
    do { // wait for any existing listener invocations to return
        struct timespec ts = { 1, 0 }; // one second
        (void)msleep(&gListenerInvocations,      // wait channel
                     NULL,                       // mutex
                     PUSER,                      // priority
                     "c_o_d_VUC::stopLogging()", // wait message
                     &ts);                       // sleep interval
    } while (gListenerInvocations > 0);
    
    return kIOReturnSuccess;
}
   
bool
c_o_d_VUC::initWithTask(task_t owningTask, void *securityID, UInt32 type)
{
    if (!super::initWithTask(owningTask, securityID , type))
        return false;
    
    if (!owningTask)
        return false;
    
    fClient = owningTask;
    fProvider = NULL;
    fDataQueue = NULL;
    fSharedMemory = NULL;
   
    return true;
}
   
IOReturn
c_o_d_VUC::registerNotificationPort(mach_port_t port, UInt32 type, UInt32 ref)
{    
    if ((!fDataQueue) || (port == MACH_PORT_NULL))
        return kIOReturnError;
   
    fDataQueue->setNotificationPort(port);
   
    return kIOReturnSuccess;
}
   
IOReturn
c_o_d_VUC::clientMemoryForType(UInt32 type, IOOptionBits *options,
                               IOMemoryDescriptor **memory)
{
    *memory = NULL;
    *options = 0;
    
    if (type == kIODefaultMemoryType) {
        if (!fSharedMemory)
            return kIOReturnNoMemory;
        fSharedMemory->retain(); // client will decrement this reference
        *memory = fSharedMemory;
        return kIOReturnSuccess;
    }
   
    // unknown memory type
    return kIOReturnNoMemory;
}
   
IOExternalMethod *
c_o_d_VUC::getTargetAndMethodForIndex(IOService **target, UInt32 index)
{
    if (index >= (UInt32)kt_kVnodeWatcherUserClientNMethods)
        return NULL;
   
    switch (index) {
    case kt_kVnodeWatcherUserClientOpen:
    case kt_kVnodeWatcherUserClientClose:
        *target = this;    
        break;
   
    default:
        *target = fProvider;
        break;
    }
        
    return (IOExternalMethod *)&sMethods[index];
}
