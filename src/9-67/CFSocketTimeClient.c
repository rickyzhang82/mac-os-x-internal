// CFSocketTimeClient.c
   
#include <CoreFoundation/CoreFoundation.h>
#include <netdb.h>
   
#define REMOTE_HOST "time.nist.gov"
   
void
dataCallBack(CFSocketRef s, CFSocketCallBackType callBackType,
             CFDataRef address, const void *data, void *info)
{
    if (data) {
        CFShow((CFDataRef)data);
        printf("%s", CFDataGetBytePtr((CFDataRef)data));
    }
}
   
int
main(int argc, char **argv)
{
    CFSocketRef         timeSocket;
    CFSocketSignature   timeSignature;
    struct sockaddr_in  remote_addr;
    struct hostent     *host;
    CFDataRef           address;
    CFOptionFlags       callBackTypes;
    CFRunLoopSourceRef  source;
    CFRunLoopRef        loop;
    struct servent     *service;
   
    if (!(host = gethostbyname(REMOTE_HOST))) {
        perror("gethostbyname");
        exit(1);
    }
   
    if (!(service = getservbyname("daytime", "tcp"))) {
        perror("getservbyname");
        exit(1);
    }
   
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(service->s_port);
    bcopy(host->h_addr, &(remote_addr.sin_addr.s_addr), host->h_length);
   
    // a CFSocketSignature structure fully specifies a CFSocket's
    // communication protocol and connection address
    timeSignature.protocolFamily = PF_INET;
    timeSignature.socketType     = SOCK_STREAM;
    timeSignature.protocol       = IPPROTO_TCP;
    address = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&remote_addr,
                           sizeof(remote_addr));
    timeSignature.address = address;
   
    // this is a variant of the read callback (kCFSocketReadCallBack): it
    // reads incoming data in the background and gives it to us packaged
    // as a CFData by invoking our callback
    callBackTypes = kCFSocketDataCallBack;
   
    timeSocket = CFSocketCreateConnectedToSocketSignature(
                     kCFAllocatorDefault, // allocator to use
                     &timeSignature,      // address and protocol
                     callBackTypes,       // activity type we are interested in
                     dataCallBack,        // call this function
                     NULL,                // context
                     10.0);               // timeout (in seconds)
   
    source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, timeSocket, 0);
    loop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(loop, source, kCFRunLoopDefaultMode);
    CFRunLoopRun();
   
    CFRelease(source);
    CFRelease(timeSocket);
    CFRelease(address);
   
    exit(0);
}
