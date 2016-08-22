// SimpleCryptoDisk.h
   
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorage.h>
   
class com_osxbook_driver_SimpleCryptoDisk : public IOStorage {
   
    OSDeclareDefaultStructors(com_osxbook_driver_SimpleCryptoDisk)
   
protected:
        IOMedia *_filteredMedia;
   
        virtual void free(void);
   
        virtual bool handleOpen(IOService    *client,
                                IOOptionBits  options,
                                void         *access);
   
        virtual bool handleIsOpen(const IOService *client) const;
        virtual void handleClose(IOService *client, IOOptionBits options);
   
public:
        virtual bool init(OSDictionary *properties = 0);
        virtual bool start(IOService *provider);
   
        virtual void read(IOService          *client,
                          UInt64              byteStart,
                          IOMemoryDescriptor *buffer,
                          IOStorageCompletion completion);
   
        virtual void write(IOService          *client,
                           UInt64              byteStart,
                           IOMemoryDescriptor *buffer,
                           IOStorageCompletion completion);
   
        virtual IOReturn synchronizeCache(IOService *client);
        virtual IOMedia *getProvider() const;
};
