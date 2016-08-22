// do_server.m
   
#import "do_common.h"
   
@interface DOServer : NSObject <ClientProtocol>
{
    float a;
    float b;
}
@end
   
// server
   
@implementation DOServer
   
- (id)init
{
    [super init];
    a = 0;
    b = 0;
    return self;
}
   
- (void)dealloc
{
    [super dealloc];
}
   
- (void)helloFromClient:(in byref id<ServerProtocol>)client
{
    NSLog([client whoAreYou]);
}
   
- (oneway void)setA:(in bycopy float)arg
{
    a = arg;
}
   
- (oneway void)setB:(in bycopy float)arg
{
    b = arg;
}
   
- (float)getSum
{
    return (float)(a + b);
}
   
@end
   
// server main program
   
int
main(int argc, char **argv)
{
    NSSocketPort *port;
    NSConnection *connection;
   
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSRunLoop *runloop = [NSRunLoop currentRunLoop];
    DOServer *server = [[DOServer alloc] init];
   
NS_DURING
    port = [[NSSocketPort alloc] initWithTCPPort:DO_DEMO_PORT];
NS_HANDLER
    NSLog(@"failed to initialize TCP port.");
    exit(1);
NS_ENDHANDLER
   
    connection = [NSConnection connectionWithReceivePort:port sendPort:nil];
    [port release];
    // vend the object
    [connection setRootObject:server];
    [server release];
    [runloop run];
    [connection release];
    [pool release];
   
    exit(0);
}
