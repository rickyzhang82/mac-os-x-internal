// NSNotificationPoster.m
   
#import <AppKit/AppKit.h>
   
#define PROGNAME "NSNotificationPoster"
   
int
main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <some name> <some value>\n", PROGNAME);
        exit(1);
    }
   
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
   
    NSString *someName = [NSString stringWithCString:argv[1]
                                            encoding:NSASCIIStringEncoding];
    NSString *someValue = [NSString stringWithCString:argv[2]
                                             encoding:NSASCIIStringEncoding];
   
    NSNotificationCenter *dnc = [NSDistributedNotificationCenter defaultCenter];
    [dnc postNotificationName:someName object:someValue];
   
    [pool release];
   
    exit(0);
}
