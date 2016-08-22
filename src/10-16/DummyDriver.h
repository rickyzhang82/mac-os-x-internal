// DummyDriver.h
   
#include <IOKit/IOService.h>
   
class com_osxbook_driver_DummyDriver : public IOService
{
    OSDeclareDefaultStructors(com_osxbook_driver_DummyDriver)
   
public:
    virtual bool       init(OSDictionary *dictionary = 0);
    virtual void       free(void);
   
    virtual bool       attach(IOService *provider);
    virtual IOService *probe(IOService *provider, SInt32 *score);
    virtual void       detach(IOService *provider);
   
    virtual bool       start(IOService *provider);
    virtual void       stop(IOService *provider);
};
