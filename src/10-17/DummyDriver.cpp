// DummyDriver.cpp
   
#include <IOKit/IOLib.h>
#include "DummyDriver.h"
   
#define super IOService
   
OSDefineMetaClassAndStructors(com_osxbook_driver_DummyDriver, IOService)
   
bool
com_osxbook_driver_DummyDriver::init(OSDictionary *dict)
{
    bool result = super::init(dict);
    IOLog("init\n");
    return result;
}
   
void
com_osxbook_driver_DummyDriver::free(void)
{
    IOLog("free\n");
    super::free();
}
   
IOService *
com_osxbook_driver_DummyDriver::probe(IOService *provider, SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("probe\n");
    return result;
}
   
bool
com_osxbook_driver_DummyDriver::start(IOService *provider)
{
    bool result = super::start(provider);
    IOLog("start\n");
    return result;
}
   
void
com_osxbook_driver_DummyDriver::stop(IOService *provider)
{
    IOLog("stop\n");
    super::stop(provider);
}
   
bool
com_osxbook_driver_DummyDriver::attach(IOService *provider)
{
    bool result = super::attach(provider);
    IOLog("attach\n");
    return result;
}
   
void
com_osxbook_driver_DummyDriver::detach(IOService *provider)
{
    IOLog("detach\n");
    super::detach(provider);
}
