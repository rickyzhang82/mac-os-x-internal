// CarbonThreadManager.c
   
#include <pthread.h>
#include <mach/mach.h>
#include <CoreServices/CoreServices.h>
   
#define MAXTHREADS 8
   
static ThreadID mainThread;
static ThreadID newThreads[MAXTHREADS] = { 0 };
   
voidPtr
threadFunction(void *threadParam)
{
    int i = (int)threadParam; 
   
    printf("thread #%d: CTM %#08lx, pthread %p, Mach %#08x\n",
           i, newThreads[i], pthread_self(), mach_thread_self());
   
    if (i == MAXTHREADS)
        YieldToThread(mainThread);
    else
        YieldToThread(newThreads[i + 1]);
   
    /* NOTREACHED */
    printf("Whoa!\n");
   
    return threadParam;
}
   
int
main()
{
    int   i;
    OSErr err = noErr;
   
    // main thread's ID
    err = GetCurrentThread(&mainThread);
   
    for (i = 0; i < MAXTHREADS; i++) {
   
        err = NewThread(
                  kCooperativeThread, // type of thread
                  threadFunction,     // thread entry function
                  (void *)i,          // function parameter
                  (Size)0,            // default stack size
                  kNewSuspend,        // options
                  NULL,               // not interested
                  &(newThreads[i]));  // newly created thread
   
        if (err || (newThreads[i] == kNoThreadID)) {
            printf("*** NewThread failed\n");
            goto out;
        }
   
        // set state of thread to "ready"
        err = SetThreadState(newThreads[i], kReadyThreadState, kNoThreadID);
    }
   
    printf("main: created %d new threads\n", i);
   
    printf("main: relinquishing control to next thread\n");
    err = YieldToThread(newThreads[0]);
   
    printf("main: back\n");
   
out:
   
    // destroy all threads
    for (i = 0; i < MAXTHREADS; i++)
        if (newThreads[i])
            DisposeThread(newThreads[i], NULL, false);
   
    exit(err);
}
