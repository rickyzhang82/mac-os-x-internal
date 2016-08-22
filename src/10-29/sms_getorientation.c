static io_connect_t dataPort = 0;
   
kern_return_t
sms_initialize(void)
{
    kern_return_t   kr;
    CFDictionaryRef classToMatch;
    io_service_t    service;
   
    // create a matching dictionary given the class name, which,
    // depends on hardware (e.g. "IOI2CMotionSensor" or "PMUMotionSensor")
    classToMatch = IOServiceMatching(kTargetIOKitClassName);
   
    // look up the IOService object (must already be registered)
    service = IOServiceGetMatchingService(kIOMasterPortDefault, classToMatch);
    if (!service)
        return KERN_FAILURE;
   
    // create a connection to the IOService object
    kr = IOServiceOpen(service,          // the IOService object
                       mach_task_self(), // the task requesting the connection
                       0,                // type of connection 
                       &dataPort);       // connection handle
   
    IOObjectRelease(service);
   
    return kr;
}
