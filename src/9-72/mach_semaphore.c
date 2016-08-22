// mach_semaphore.c
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <mach/mach.h>
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
#define PTHID() (unsigned long)(pthread_self())
   
#define SEMAPHORE_WAIT(s, n) \
    { int i; for (i = 0; i < (n); i++) { semaphore_wait((s)); } }
   
void *
start_routine(void *semaphores)
{
    semaphore_t *sem = (semaphore_t *)semaphores;
   
    semaphore_signal(sem[1]);
    printf("thread: %lx about to decrement semaphore count\n", PTHID());
    semaphore_wait(sem[0]);
    printf("thread: %lx succeeded in decrementing semaphore count\n", PTHID());
    semaphore_signal(sem[1]);
    return (void *)0;
}
   
int
main(void)
{
    pthread_t     pthread1, pthread2, pthread3;
    semaphore_t   sem[2] = { 0 };
    kern_return_t kr;
   
    setbuf(stdout, NULL);
   
    kr = semaphore_create(mach_task_self(), &sem[0], SYNC_POLICY_FIFO, 0);
    OUT_ON_MACH_ERROR("semaphore_create", kr);
   
    kr = semaphore_create(mach_task_self(), &sem[1], SYNC_POLICY_FIFO, 0);
    OUT_ON_MACH_ERROR("semaphore_create", kr);
   
    (void)pthread_create(&pthread1, (const pthread_attr_t *)0,
                         start_routine, (void *)sem);
    printf("created thread1=%lx\n", (unsigned long)pthread1);
   
    (void)pthread_create(&pthread2, (const pthread_attr_t *)0,
                         start_routine, (void *)sem);
    printf("created thread2=%lx\n", (unsigned long)pthread2);
   
    (void)pthread_create(&pthread3, (const pthread_attr_t *)0,
                         start_routine, (void *)sem);
    printf("created thread3=%lx\n", (unsigned long)pthread3);
   
    // wait until all three threads are ready
    SEMAPHORE_WAIT(sem[1], 3);
   
    printf("main: about to signal thread3\n");
    semaphore_signal_thread(sem[0], pthread_mach_thread_np(pthread3));
   
    // wait for thread3 to sem_signal()
    semaphore_wait(sem[1]);
   
    printf("main: about to signal all threads\n");
    semaphore_signal_all(sem[0]);
   
    // wait for thread1 and thread2 to sem_signal()
    SEMAPHORE_WAIT(sem[1], 2);
   
out:
    if (sem[0])
        semaphore_destroy(mach_task_self(), sem[0]);
    if (sem[1])
        semaphore_destroy(mach_task_self(), sem[1]);
    
    exit(kr);
}
