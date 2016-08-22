// sched_pri_shift.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
   
// defined in osfmk/kern/sched.h
#define BASEPRI_DEFAULT 31
#define SCHED_TICK_SHIFT 3
   
void
clock_interval_to_absolutetime_interval(uint32_t interval,
                                        uint32_t scale_factor,
                                        uint64_t *result)
{
    uint64_t t64;
    uint32_t divisor, rtclock_sec_divisor;
    uint64_t nanosecs = (uint64_t)interval * scale_factor;
    mach_timebase_info_data_t tbinfo;
   
    (void)mach_timebase_info(&tbinfo);
   
    // see timebase_callback() [osfmk/ppc/rtclock.c]
    rtclock_sec_divisor = tbinfo.denom / (tbinfo.numer / NSEC_PER_SEC);
   
    *result = (t64 = nanosecs / NSEC_PER_SEC) * (divisor = rtclock_sec_divisor);
    nanosecs -= (t64 * NSEC_PER_SEC);
    *result += (nanosecs * divisor) / NSEC_PER_SEC;
}
   
int
main(void)
{
    uint64_t abstime;
    uint32_t sched_pri_shift;
    uint32_t sched_tick_interval;
   
    clock_interval_to_absolutetime_interval(USEC_PER_SEC >> SCHED_TICK_SHIFT,
                                            NSEC_PER_USEC, &abstime);
    sched_tick_interval = abstime; // lvalue is 32-bit
    abstime = (abstime * 5) / 3;
    for (sched_pri_shift = 0; abstime > BASEPRI_DEFAULT; ++sched_pri_shift)
        abstime >>= 1;
   
    printf("sched_tick_interval = %u\n", sched_tick_interval);
    printf("sched_pri_shift = %u\n", sched_pri_shift);
   
    exit(0);
}
