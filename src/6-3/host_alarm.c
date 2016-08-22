// host_alarm.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/clock.h>
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }
   
// Structure for the IPC message we will receive from the clock
typedef struct msg_format_recv_s {
    mach_msg_header_t  header;
    int                data;
    mach_msg_trailer_t trailer;
} msg_format_recv_t;
   
int
main()
{
    kern_return_t     kr;
    clock_serv_t      clk_system;
    mach_timespec_t   alarm_time;
    clock_reply_t     alarm_port;
    struct timeval    t1, t2;
    msg_format_recv_t message;
    mach_port_t       mytask;
   
    // The C library optimized this call by returning the task port's value
    // that it caches in the mach_task_self_ variable
    mytask = mach_task_self();
   
    kr = host_get_clock_service(mach_host_self(), SYSTEM_CLOCK,
                                (clock_serv_t *)&clk_system);
    OUT_ON_MACH_ERROR("host_get_clock_service", kr);
   
    // Let us set the alarm to ring after 2.5 seconds
    alarm_time.tv_sec = 2;
    alarm_time.tv_nsec = 50000000;
   
    // Allocate a port (specifically, get receive right for the new port)
    // We will use this port to receive the alarm message from the clock
    kr = mach_port_allocate(
             mytask,                  // the task acquiring the port right
             MACH_PORT_RIGHT_RECEIVE, // type of right
             &alarm_port);            // task's name for the port right
    OUT_ON_MACH_ERROR("mach_port_allocate", kr);
   
    gettimeofday(&t1, NULL);
   
    // Set the alarm
    kr = clock_alarm(clk_system,    // the clock to use
                     TIME_RELATIVE, // how to interpret alarm time
                     alarm_time,    // the alarm time
                     alarm_port);   // this port will receive the alarm message
    OUT_ON_MACH_ERROR("clock_alarm", kr);
   
    printf("Current time %ld s + %d us\n"
           "Setting alarm to ring after %d s + %d ns\n",
           t1.tv_sec, t1.tv_usec, alarm_time.tv_sec, alarm_time.tv_nsec);
   
    // Wait to receive the alarm message (we will block here)
    kr = mach_msg(&(message.header),       // the message buffer
                  MACH_RCV_MSG,            // message option bits
                  0,                       // send size (we are receiving, so 0)
                  message.header.msgh_size,// receive limit
                  alarm_port,              // receive right
                  MACH_MSG_TIMEOUT_NONE,   // no timeout
                  MACH_PORT_NULL);         // no timeout notification port
    // We should have received an alarm message at this point
    gettimeofday(&t2, NULL);
    OUT_ON_MACH_ERROR("mach_msg", kr);
   
    if (t2.tv_usec < t1.tv_usec) {
        t1.tv_sec += 1;
        t1.tv_usec -= 1000000;
    }
   
    printf("\nCurrent time %ld s + %d us\n", t2.tv_sec, t2.tv_usec);
    printf("Alarm rang after %ld s + %d us\n", (t2.tv_sec - t1.tv_sec),
          (t2.tv_usec - t1.tv_usec));
   
out:
    mach_port_deallocate(mytask, clk_system);
   
    // Release user reference for the receive right we created
    mach_port_deallocate(mytask, alarm_port);
   
    exit(0);
}
