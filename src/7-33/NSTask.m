// NSTask.m
   
#import <Foundation/Foundation.h>
   
#define TASK_PATH "/bin/sleep"
#define TASK_ARGS "15"
   
int
main()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
   
    NSTask *newTask;
    int     status;
   
    // allocate and initialize an NSTask object
    newTask = [[NSTask alloc] init];
   
    // set the executable for the program to be run
    [newTask setLaunchPath:@TASK_PATH];
   
    // set the command arguments that will be used to launch the program
    [newTask setArguments:[NSArray arrayWithObject:@TASK_ARGS]];
   
    // launch the program -- a new process will be created
    [newTask launch];
   
    NSLog(@"waiting for new task to exit\n");
    [newTask waitUntilExit];
   
    // fetch the value returned by the exiting program
    status = [newTask terminationStatus];
   
    NSLog(@"new task exited with status %d\n", status);
   
    [pool release];
   
    exit(0);
}
