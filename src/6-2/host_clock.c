// host_clock.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/clock.h>
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
int
main()
{
    kern_return_t          kr;
    host_name_port_t       myhost;
    clock_serv_t           clk_system, clk_calendar, clk_realtime;
    natural_t              attribute[4];
    mach_msg_type_number_t count;
    mach_timespec_t        timespec;
    struct timeval         t;
   
    myhost = mach_host_self();
   
    // Get a send right to the system clock's name port
    kr = host_get_clock_service(myhost,  SYSTEM_CLOCK,
                                (clock_serv_t *)&clk_system);
    OUT_ON_MACH_ERROR("host_get_clock_service", kr);
   
    // Get a send right to the calendar clock's name port
    kr = host_get_clock_service(myhost, CALENDAR_CLOCK,
                                (clock_serv_t *)&clk_calendar);
    OUT_ON_MACH_ERROR("host_get_clock_service", kr);
   
    // Get a send right to the real-time clock's name port
    kr = host_get_clock_service(myhost, REALTIME_CLOCK,
                                (clock_serv_t *)&clk_realtime);
    OUT_ON_MACH_ERROR("host_get_clock_service", kr);
   
    //// System clock
    count = sizeof(attribute)/sizeof(natural_t);
    // Get the clock's resolution in nanoseconds
    kr = clock_get_attributes(clk_system, CLOCK_GET_TIME_RES,
                              (clock_attr_t)attribute, &count);
    OUT_ON_MACH_ERROR("clock_get_attributes", kr);
    // Get the current time
    kr = clock_get_time(clk_system, &timespec);
    OUT_ON_MACH_ERROR("clock_get_time", kr);
    printf("System clock  : %u s + %u ns (res %u ns)\n",
           timespec.tv_sec, timespec.tv_nsec, attribute[0]);
   
    //// Real-time clock
    count = sizeof(attribute)/sizeof(natural_t);
    kr = clock_get_attributes(clk_realtime, CLOCK_GET_TIME_RES,
                              (clock_attr_t)&attribute, &count);
    OUT_ON_MACH_ERROR("clock_get_attributes", kr);
    kr = clock_get_time(clk_realtime, &timespec);
    OUT_ON_MACH_ERROR("clock_get_time", kr);
    printf("Realtime clock: %u s + %u ns (res %u ns)\n",
           timespec.tv_sec, timespec.tv_nsec, attribute[0]);
   
    //// Calendar clock
    count = sizeof(attribute)/sizeof(natural_t);
    kr = clock_get_attributes(clk_calendar, CLOCK_GET_TIME_RES,
                              (clock_attr_t)&attribute, &count);
    OUT_ON_MACH_ERROR("clock_get_attributes", kr);
    kr = clock_get_time(clk_calendar, &timespec);
    gettimeofday(&t, NULL);
    OUT_ON_MACH_ERROR("clock_get_time", kr);
    printf("Calendar clock: %u s + %u ns (res %u ns)\n",
           timespec.tv_sec, timespec.tv_nsec, attribute[0]);
   
    printf("gettimeofday  : %ld s + %d us\n", t.tv_sec, t.tv_usec);
   
out:
    // Should deallocate ports here for cleanliness
    mach_port_deallocate(mach_task_self(), myhost);
    mach_port_deallocate(mach_task_self(), clk_calendar);
    mach_port_deallocate(mach_task_self(), clk_system);
    mach_port_deallocate(mach_task_self(), clk_realtime);
   
    exit(0);
}
