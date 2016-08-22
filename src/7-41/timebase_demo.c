// timebase_demo.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
   
#define DEFAULT_SLEEP_TIME 1
#define MAXIMUM_SLEEP_TIME 60
   
int
main(int argc, char **argv)
{
    kern_return_t kr;
    u_int64_t     t1, t2, diff;
    double        abs2clock;
    int           sleeptime = DEFAULT_SLEEP_TIME;
   
    mach_timebase_info_data_t info;
   
    kr = mach_timebase_info(&info);
    if (kr != KERN_SUCCESS) {
        mach_error("mach_timebase_info:", kr);
        exit(kr);
    }
   
    if (argc == 2) {
        sleeptime = atoi(argv[1]);
        if ((sleeptime < 0) || (sleeptime > MAXIMUM_SLEEP_TIME))
            sleeptime = DEFAULT_SLEEP_TIME;
    } 
   
    t1 = mach_absolute_time();
    sleep(sleeptime);
    t2 = mach_absolute_time();
    diff = t2 - t1;
   
    printf("slept for %d seconds of clock time\n", sleeptime);
    printf("TB increments = %llu increments\n", diff);
    printf("absolute-to-clock conversion factor = (%u/%u) ns/increment\n",
           info.numer, info.denom);
    printf("sleeping time according to TB\n");
   
    abs2clock = (double)info.numer/(double)info.denom;
    abs2clock *= (double)diff;
   
    printf("\t= %llu increments x (%u/%u) ns/increment\n\t= %f ns\n\t= %f s\n",
           diff, info.numer, info.denom,
           abs2clock, abs2clock/(double)1000000000);
   
    exit(0);
}
