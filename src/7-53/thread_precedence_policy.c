// thread_precedence_policy.c
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/param.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
   
#define PROGNAME "thread_precedence_policy"
   
void
usage(void)
{
    fprintf(stderr, "usage: %s <thread1 importance> <thread2 importance>\n"
                    "       where %d <= importance <= %d\n",
            PROGNAME, -MAXPRI, MAXPRI);
    exit(1);
}
   
void *
adder(void *arg)
{
    unsigned long long *ctr = (unsigned long long *)arg;
    sleep(1);
    while (1)
        (*ctr)++;
   
    return NULL;
}
   
int
main(int argc, char **argv)
{
    int                ret, imp1, imp2;
    kern_return_t      kr;
    pthread_t          t1, t2;
    unsigned long long ctr1 = 0, ctr2 = 0;
   
    thread_precedence_policy_data_t policy;
   
    if (argc != 3)
        usage();
   
    imp1 = atoi(argv[1]);
    imp2 = atoi(argv[2]);
    if ((abs(imp1) > MAXPRI) || (abs(imp2) > MAXPRI))
        usage();
   
    ret = pthread_create(&t1, (pthread_attr_t *)0, adder, (void *)&ctr1);
    ret = pthread_create(&t2, (pthread_attr_t *)0, adder, (void *)&ctr2);
   
    policy.importance = imp1;
    kr = thread_policy_set(pthread_mach_thread_np(t1),
                           THREAD_PRECEDENCE_POLICY,
                           (thread_policy_t)&policy,
                           THREAD_PRECEDENCE_POLICY_COUNT);
   
    policy.importance = imp2;
    kr = thread_policy_set(pthread_mach_thread_np(t2),
                           THREAD_PRECEDENCE_POLICY,
                           (thread_policy_t)&policy,
                           THREAD_PRECEDENCE_POLICY_COUNT);
   
    ret = pthread_detach(t1);
    ret = pthread_detach(t2);
   
    sleep(10);
   
    printf("ctr1=%llu ctr2=%llu\n", ctr1, ctr2);
   
    exit(0);
}
