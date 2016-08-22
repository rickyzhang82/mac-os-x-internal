// CFMessagePortClient.c
   
#include <CoreFoundation/CoreFoundation.h>
   
#define REMOTE_NAME "com.osxbook.CFMessagePort.server"
   
int
main(void)
{
    SInt32           status;
    CFMessagePortRef remotePort;
    CFDataRef        sendData;
    const UInt8      bytes[] = { 1, 2, 3, 4 };
   
    sendData = CFDataCreate(kCFAllocatorDefault, bytes,
                            sizeof(bytes)/sizeof(UInt8));
    if (sendData == NULL) {
        fprintf(stderr, "*** CFDataCreate\n");
        exit(1);
    }
   
    remotePort = CFMessagePortCreateRemote(kCFAllocatorDefault,
                                           CFSTR(REMOTE_NAME));
    if (remotePort == NULL) {
        CFRelease(sendData);
        fprintf(stderr, "*** CFMessagePortCreateRemote\n");
        exit(1);
    }
   
    status = CFMessagePortSendRequest(
                 remotePort,     // message port to which data should be sent
                 (SInt32)0x1234, // msgid, an arbitrary integer value
                 sendData,       // data
                 5.0,            // send timeout
                 5.0,            // receive timeout
                 NULL,           // reply mode (no reply expected or desired)
                 NULL);          // reply data
   
    if (status != kCFMessagePortSuccess)
        fprintf(stderr, "*** CFMessagePortSendRequest: error %ld.\n", status);
    else
        printf("message sent\n");
   
    CFRelease(sendData);
    CFRelease(remotePort);
   
    exit(0);
}
