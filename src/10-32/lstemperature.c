// lstemperature.c
   
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
   
#define kIOPPluginCurrentValueKey "current-value" // current measured value
#define kIOPPluginLocationKey     "location"      // readable description
#define kIOPPluginTypeKey         "type"          // sensor/control type
#define kIOPPluginTypeTempSensor  "temperature"   // desired type value
   
// macro to convert sensor temperature format (16.16) to integer (Celsius)
#define SENSOR_TEMP_FMT_C(x) (double)((x) >> 16)
   
// macro to convert sensor temperature format (16.16) to integer (Fahrenheit)
#define SENSOR_TEMP_FMT_F(x) \
    (double)((((double)((x) >> 16) * (double)9) / (double)5) + (double)32)
   
void
printTemperatureSensor(const void *sensorDict, CFStringEncoding encoding)
{
    SInt32      currentValue;
    CFNumberRef sensorValue;
    CFStringRef sensorType, sensorLocation;
   
    if (!CFDictionaryGetValueIfPresent((CFDictionaryRef)sensorDict,
                                       CFSTR(kIOPPluginTypeKey),
                                       (void *)&sensorType))
        return;
   
    if (CFStringCompare(sensorType, CFSTR(kIOPPluginTypeTempSensor), 0) !=
                        kCFCompareEqualTo) // we handle only temperature sensors
        return;
   
    sensorLocation = CFDictionaryGetValue((CFDictionaryRef)sensorDict,
                                          CFSTR(kIOPPluginLocationKey));
   
    sensorValue = CFDictionaryGetValue((CFDictionaryRef)sensorDict,
                                       CFSTR(kIOPPluginCurrentValueKey));
    (void)CFNumberGetValue(sensorValue, kCFNumberSInt32Type,
                           (void *)&currentValue);
   
    printf("%24s %7.1f C %9.1f F\n",
           // see documentation for CFStringGetCStringPtr() caveat
           CFStringGetCStringPtr(sensorLocation, encoding),
           SENSOR_TEMP_FMT_C(currentValue),
           SENSOR_TEMP_FMT_F(currentValue));
}
   
int
main(void)
{
    kern_return_t          kr;
    io_iterator_t          io_hw_sensors;
    io_service_t           io_hw_sensor;
    CFMutableDictionaryRef sensor_properties;
    CFStringEncoding       systemEncoding = CFStringGetSystemEncoding();
   
    kr = IOServiceGetMatchingServices(kIOMasterPortDefault,
             IOServiceNameMatching("IOHWSensor"), &io_hw_sensors);
   
    while ((io_hw_sensor = IOIteratorNext(io_hw_sensors))) {
        kr = IORegistryEntryCreateCFProperties(io_hw_sensor, &sensor_properties,
                 kCFAllocatorDefault, kNilOptions);
        if (kr == KERN_SUCCESS)
            printTemperatureSensor(sensor_properties, systemEncoding);
   
        CFRelease(sensor_properties);
        IOObjectRelease(io_hw_sensor);
    }
   
    IOObjectRelease(io_hw_sensors);
   
    exit(kr);
}
