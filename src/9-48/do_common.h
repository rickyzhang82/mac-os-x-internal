// do_common.h
   
#import <Foundation/Foundation.h>
#include <sys/socket.h>
   
#define DO_DEMO_PORT 12345
#define DO_DEMO_HOST "localhost"
   
@protocol ClientProtocol
   
- (void)setA:(float)arg;
- (void)setB:(float)arg;
- (float)getSum;
- (void)helloFromClient:(id)client;
   
@end
   
@protocol ServerProtocol
   
- (bycopy NSString *)whoAreYou;
   
@end
