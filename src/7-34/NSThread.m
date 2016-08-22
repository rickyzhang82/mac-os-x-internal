// NSThread.m
   
#import <Foundation/Foundation.h>
   
@interface NSThreadController : NSObject
   
{
    unsigned long long sum1;
    unsigned long long sum2;
}
   
- (void)thread1:(id)arg;
- (void)thread2:(id)arg;
- (unsigned long long)get_sum1;
- (unsigned long long)get_sum2;
   
@end
   
@implementation NSThreadController
   
- (unsigned long long)get_sum1
{
    return sum1;
}
   
- (unsigned long long)get_sum2
{
    return sum2;
}
   
- (void)thread1:(id)arg
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSThread setThreadPriority:0.0];
    sum1 = 0;
    printf("thread1: running\n");
    for (;;)
        sum1++;
    [pool release];
}
   
- (void)thread2:(id)arg
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSThread setThreadPriority:1.0];
    sum2 = 0;
    printf("thread2: running\n");
    for (;;)
       sum2++;
    [pool release];
}
   
@end
   
int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSTimeInterval secs = 5;
    NSDate *sleepForDate = [NSDate dateWithTimeIntervalSinceNow:secs];
   
    NSThreadController *T = [[NSThreadController alloc] init];
    [NSThread detachNewThreadSelector:@selector(thread1:)
                             toTarget:T
                           withObject:nil];
   
    [NSThread detachNewThreadSelector:@selector(thread2:)
                             toTarget:T
                           withObject:nil];
   
    printf("main: sleeping for %f seconds\n", secs);
    [NSThread sleepUntilDate:sleepForDate];
   
    printf("sum1 = %lld\n", [T get_sum1]);
    printf("sum2 = %lld\n", [T get_sum2]);
   
    [pool release];
   
    exit(0);
}
