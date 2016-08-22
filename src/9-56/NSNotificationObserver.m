// NSNotificationObserver.m
   
#import <AppKit/AppKit.h>
   
@interface DummyNotificationHandler : NSObject
{
    NSNotificationCenter *dnc;
}
   
- (void)defaultNotificationHandler:(NSNotification *)notification;
   
@end
   
@implementation DummyNotificationHandler
   
- (id)init
{
    [super init];
    dnc = [NSDistributedNotificationCenter defaultCenter];
    [dnc addObserver:self
           selector:@selector(defaultNotificationHandler:)
               name:nil
             object:nil];
    return self;
}
   
- (void)dealloc
{
    [dnc removeObserver:self name:nil object:nil];
    [super dealloc];
}
   
- (void)defaultNotificationHandler:(NSNotification *)notification
{
    NSLog(@"name=%@ value=%@", [notification name], [notification object]);
}
   
@end
   
int
main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSRunLoop *runloop = [NSRunLoop currentRunLoop];
    [[DummyNotificationHandler alloc] init];
    [runloop run];
    [pool release];
    exit(0);
}
