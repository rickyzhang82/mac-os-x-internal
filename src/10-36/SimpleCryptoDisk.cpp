// SimpleCryptoDisk.cpp
   
#include <IOKit/assert.h>
#include <IOKit/IOLib.h>
#include "SimpleCryptoDisk.h"
   
#define super IOStorage
   
OSDefineMetaClassAndStructors(com_osxbook_driver_SimpleCryptoDisk, IOStorage)
   
// Context structure for our read/write completion routines
typedef struct {
    IOMemoryDescriptor *buffer;
    IOMemoryDescriptor *bufferRO;
    IOMemoryDescriptor *bufferWO;
    void               *memory;
    vm_size_t           size;
    IOStorageCompletion completion;
} SimpleCryptoDiskContext;
   
// Internal functions
static void fixBufferUserRead(IOMemoryDescriptor *buffer);
static void fixBufferUserWrite(IOMemoryDescriptor *bufferR,
                               IOMemoryDescriptor *bufferW);
static void SCDReadWriteCompletion(void *target, void *parameter,
                                   IOReturn status, UInt64 actualByteCount);
   
bool
com_osxbook_driver_SimpleCryptoDisk::init(OSDictionary *properties)
{    
    if (super::init(properties) == false)
        return false;
   
    _filteredMedia = 0;
    
    return true;
}
   
void
com_osxbook_driver_SimpleCryptoDisk::free(void)
{
    if (_filteredMedia)
        _filteredMedia->release();
        
    super::free();
}
   
bool
com_osxbook_driver_SimpleCryptoDisk::start(IOService *provider)
{
    IOMedia *media = (IOMedia *)provider;
    
    assert(media);
    
    if (super::start(provider) == false)
        return false;
        
    IOMedia *newMedia = new IOMedia;
    if (!newMedia)
        return false;
    
    if (!newMedia->init(
            0,                              // media offset in bytes
            media->getSize(),               // media size in bytes
            media->getPreferredBlockSize(), // natural block size in bytes
            media->isEjectable(),           // is media ejectable?
            false,                          // is it the whole disk?
            media->isWritable(),            // is media writable?
            "Apple_HFS")) {                 // hint of media's contents
        newMedia->release();
        newMedia = 0;
        return false;
    }
   
    UInt32 partitionID = 1;
    char name[32];
            
    // Set a name for this partition.
    sprintf(name, "osxbook_HFS %ld", partitionID);
    newMedia->setName(name);
        
    // Set a location value (partition #) for this partition.
    char location[32];
    sprintf(location, "%ld", partitionID);
    newMedia->setLocation(location);
        
    _filteredMedia = newMedia;
    newMedia->attach(this);
    newMedia->registerService();
    
    return true;
}
   
bool
com_osxbook_driver_SimpleCryptoDisk::handleOpen(IOService   *client,
                                                IOOptionBits options,
                                                void        *argument)
{
    return getProvider()->open(this, options, (IOStorageAccess)argument);
}
   
bool
com_osxbook_driver_SimpleCryptoDisk::handleIsOpen(const IOService *client) const
{
    return getProvider()->isOpen(this);
}
   
void
com_osxbook_driver_SimpleCryptoDisk::handleClose(IOService   *client,
                                                 IOOptionBits options)
{
    getProvider()->close(this, options);
}
   
IOReturn
com_osxbook_driver_SimpleCryptoDisk::synchronizeCache(IOService *client)
{
    return getProvider()->synchronizeCache(this);
}
   
IOMedia *
com_osxbook_driver_SimpleCryptoDisk::getProvider(void) const
{
    return (IOMedia *)IOService::getProvider();
}
   
void
com_osxbook_driver_SimpleCryptoDisk::read(IOService          *client,
                                          UInt64              byteStart,
                                          IOMemoryDescriptor *buffer,
                                          IOStorageCompletion completion)
{
    SimpleCryptoDiskContext *context =
        (SimpleCryptoDiskContext *)IOMalloc(sizeof(SimpleCryptoDiskContext));
    context->buffer     = buffer;
    context->bufferRO   = NULL;
    context->bufferWO   = NULL;
    context->memory     = NULL;
    context->size       = (vm_size_t)0;
   
    // Save original completion function and insert our own.
    context->completion  = completion;
    completion.action    = (IOStorageCompletionAction)&SCDReadWriteCompletion;
    completion.target    = (void *)this;
    completion.parameter = (void *)context;
        
    // Hand over to the provider.
    return getProvider()->read(this, byteStart, buffer, completion);
}
   
void
com_osxbook_driver_SimpleCryptoDisk::write(IOService          *client,
                                           UInt64              byteStart,
                                           IOMemoryDescriptor *buffer,
                                           IOStorageCompletion completion)
{
    // The buffer passed to this function would have been created with a
    // direction of kIODirectionOut. We need a new buffer that is created
    // with a direction of kIODirectionIn to store the modified contents
    // of the original buffer.
   
    // Determine the original buffer's length.
    IOByteCount length = buffer->getLength();
   
    // Allocate memory for a new (temporary) buffer. Note that we would be
    // passing this modified buffer (instead of the original) to our
    // provider's write function. We need a kIODirectionOut "pointer",
    // a new memory descriptor referring to the same memory, that we shall
    // pass to the provider's write function.
    void *memory = IOMalloc(length);
   
    // We use this descriptor to modify contents of the original buffer.
    IOMemoryDescriptor *bufferWO = 
        IOMemoryDescriptor::withAddress(memory, length, kIODirectionIn);
   
    // We use this descriptor as the buffer argument in the provider's write().
    IOMemoryDescriptor *bufferRO =
        IOMemoryDescriptor::withSubRange(bufferWO, 0, length, kIODirectionOut);
   
    SimpleCryptoDiskContext *context =
        (SimpleCryptoDiskContext *)IOMalloc(sizeof(SimpleCryptoDiskContext));
    context->buffer      = buffer;
    context->bufferRO    = bufferRO;
    context->bufferWO    = bufferWO;
    context->memory      = memory;
    context->size        = (vm_size_t)length;
   
    // Save the original completion function and insert our own.
    context->completion  = completion;
    completion.action    = (IOStorageCompletionAction)&SCDReadWriteCompletion;
    completion.target    = (void *)this;
    completion.parameter = (void *)context;
    
    // Fix buffer contents (apply simple "encryption").
    fixBufferUserWrite(buffer, bufferWO);
   
    // Hand over to the provider.
    return getProvider()->write(this, byteStart, bufferRO, completion);
}
   
static void
fixBufferUserRead(IOMemoryDescriptor *buffer)
{
    IOByteCount i, j;
    IOByteCount length, count;
    UInt64      byteBlock[64];
    
    assert(buffer);
    
    length = buffer->getLength();
    assert(!(length % 512));
    length /= 512;
    
    buffer->prepare(kIODirectionOutIn);
    
    for (i = 0; i < length; i++) {
        count = buffer->readBytes(i * 512, (UInt8 *)byteBlock, 512);
        for (j = 0; j < 64; j++)
            byteBlock[j] = ~(byteBlock[j]);
        count = buffer->writeBytes(i * 512, (UInt8 *)byteBlock, 512);
    }
    
    buffer->complete();
    
    return;
}
   
static void
fixBufferUserWrite(IOMemoryDescriptor *bufferR, IOMemoryDescriptor *bufferW)
{
    IOByteCount i, j;
    IOByteCount length, count;
    UInt64      byteBlock[64];
    
    assert(bufferR);
    assert(bufferW);
    
    length = bufferR->getLength();
    assert(!(length % 512));
    length /= 512;
    
    bufferR->prepare(kIODirectionOut);
    bufferW->prepare(kIODirectionIn);
    
    for (i = 0; i < length; i++) {
        count = bufferR->readBytes(i * 512, (UInt8 *)byteBlock, 512);
        for (j = 0; j < 64; j++)
            byteBlock[j] = ~(byteBlock[j]);
        count = bufferW->writeBytes(i * 512, (UInt8 *)byteBlock, 512);
    }
    
    bufferW->complete();
    bufferR->complete();
    
    return;
}
   
static void
SCDReadWriteCompletion(void    *target,
                       void    *parameter,
                       IOReturn status,
                       UInt64   actualByteCount)
{
    SimpleCryptoDiskContext *context = (SimpleCryptoDiskContext *)parameter;
   
    if (context->bufferWO == NULL) { // this was a read
   
        // Fix buffer contents (apply simple "decryption").
        fixBufferUserRead(context->buffer);
   
    } else { // This was a write.
   
        // Release temporary memory descriptors and free memory that we had
        // allocated in the write call.
        (context->bufferRO)->release();
        (context->bufferWO)->release();
        IOFree(context->memory, context->size);
    }
    
    // Retrieve the original completion routine.
    IOStorageCompletion completion = context->completion;
   
    IOFree(context, sizeof(SimpleCryptoDiskContext));
   
    // Run the original completion routine, if any.
    if (completion.action)
        (*completion.action)(completion.target, completion.parameter, status,
                             actualByteCount);
}
