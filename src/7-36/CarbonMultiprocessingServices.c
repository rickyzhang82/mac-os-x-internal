// CarbonMultiprocessingServices.c
   
#include <pthread.h>
#include <CoreServices/CoreServices.h>
   
OSStatus
taskFunction(void *param)
{
    printf("taskFunction: I am an MP Services task\n");
    printf("taskFunction: my task ID is %#x\n", (int)MPCurrentTaskID());
    printf("taskFunction: my pthread ID is %p\n", pthread_self());
    return noErr;
}
   
int
main()
{
    MPQueueID queue;
    UInt32    param1, param2;
    UInt32    tParam1, tParam2;
    OSStatus  status;
    MPTaskID  task;
   
    // check for availability
    if (MPLibraryIsLoaded()) {
        printf("MP Services initialized\n");
        printf("MP Services version %d.%d.%d.%d\n",
               MPLibrary_MajorVersion, MPLibrary_MinorVersion,
               MPLibrary_Release, MPLibrary_DevelopmentRevision);
        printf("%d processors available\n\n", (int)MPProcessorsScheduled());
    } else
        printf("MP Services not available\n");
   
    printf("main: currently executing task is %#x\n", (int)MPCurrentTaskID());
   
    // create a notification queue
    status = MPCreateQueue(&queue);
    if (status != noErr) {
        printf("failed to create MP notification queue (error %lu)\n", status);
        exit(1);
    }
   
    tParam1 = 1234;
    tParam2 = 5678;
   
    printf("main: about to create new task\n");
    printf("main: my pthread ID is %p\n", pthread_self());
   
    // create an MP Services task
    status = MPCreateTask(taskFunction, // pointer to the task function
                          (void *)0,    // parameter to pass to the task
                          (ByteCount)0, // stack size (0 for default)
                          queue,        // notify this queue upon termination
                          &tParam1,     // termination parameter 1
                          &tParam2,     // termination parameter 2
                          kMPCreateTaskValidOptionsMask,
                          &task);       // ID of the newly created task
    if (status != noErr) {
        printf("failed to create MP Services task (error %lu)\n", status);
        goto out;
    }
   
    printf("main: created new task %#08x, now waiting\n", (int)task);
   
    // wait for the task to be terminated
    status = MPWaitOnQueue(queue, (void *)&param1, (void *)&param2,
                           NULL, kDurationForever);
   
    printf("main: task terminated (param1 %lu, param2 %lu)\n",
           tParam1, tParam2);
   
out:
    if (queue)
        MPDeleteQueue(queue);
   
    exit(0);
}
