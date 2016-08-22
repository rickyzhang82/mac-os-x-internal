// do_client.m
   
#import "do_common.h"
   
@interface DOClient : NSObject <ServerProtocol>
{
    id proxy;
}
   
- (NSString *)whoAreYou;
- (void)cleanup;
- (void)connect;
- (void)doTest;
   
@end
   
// client
   
@implementation DOClient
   
- (void)dealloc
{
    [self cleanup];
    [super dealloc];
}
   
- (void)cleanup
{
    if (proxy) {
        NSConnection *connection = [proxy connectionForProxy];
        [connection invalidate];
        [proxy release];
        proxy = nil;
    }
}
   
- (NSString *)whoAreYou
{
    return @"I am a DO client.";
}
   
- (void)connect
{
    NSSocketPort *port;
    NSConnection *connection;
   
    port = [[NSSocketPort alloc] initRemoteWithTCPPort:DO_DEMO_PORT
                                                  host:@DO_DEMO_HOST];
    connection = [NSConnection connectionWithReceivePort:nil sendPort:port];
    [connection setReplyTimeout:5];
    [connection setRequestTimeout:5];
    [port release];
NS_DURING
    proxy = [[connection rootProxy] retain];
    [proxy setProtocolForProxy:@protocol(ClientProtocol)];
    [proxy helloFromClient:self];
NS_HANDLER
    [self cleanup];
NS_ENDHANDLER
}
   
- (void)doTest
{
    [proxy setA:4.0];
    [proxy setB:9.0];
    float result = [proxy getSum];
    NSLog(@"%f", result);
}
   
@end
   
// client main program
   
int
main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    DOClient *client = [[DOClient alloc] init];
    [client connect];
    [client doTest];
    [client release];
    [pool release];
   
    exit(0);
}
