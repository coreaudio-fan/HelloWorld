#import <Foundation/Foundation.h>
#include "HelloWorld_objc.h"

void	hello_objc(void) {
	NSString *msg = @"Hello from Objective-C";
	fprintf(stdout, "%s\n", msg.UTF8String);
}
