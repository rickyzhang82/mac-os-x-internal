// host_statistics.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
// Wrapper function with error checking
kern_return_t
do_host_statistics(host_name_port_t        host,
                   host_flavor_t           flavor,
                   host_info_t             info,
                   mach_msg_type_number_t *count)
{
    kern_return_t kr;
   
    kr = host_statistics(host,              // control port for the host
                         flavor,            // type of statistics desired
                         (host_info_t)info, // out buffer
                         count);            // in/out size of buffer
    if (kr != KERN_SUCCESS) {
        (void)mach_port_deallocate(mach_task_self(), host);
        mach_error("host_info:", kr);
        exit(1);
    }
   
    return kr;
}
   
int
main()
{
    kern_return_t             kr;
    host_name_port_t          host;
    mach_msg_type_number_t    count;
    vm_size_t                 page_size;
    host_load_info_data_t     load_info;
    host_cpu_load_info_data_t cpu_load_info;
    vm_statistics_data_t      vm_stat;
   
    host = mach_host_self();
   
    count = HOST_LOAD_INFO_COUNT;
    // Get system loading statistics
    kr = do_host_statistics(host, HOST_LOAD_INFO, (host_info_t)&load_info,
                            &count);
   
    count = HOST_VM_INFO_COUNT;
    // Get virtual memory statistics
    kr = do_host_statistics(host, HOST_VM_INFO, (host_info_t)&vm_stat, &count);
   
    count = HOST_CPU_LOAD_INFO_COUNT;
    // Get CPU load statistics
    kr = do_host_statistics(host, HOST_CPU_LOAD_INFO,
                            (host_info_t)&cpu_load_info, &count);
   
    kr = host_page_size(host, &page_size);
   
    printf("Host statistics:\n");
   
    // (average # of runnable processes) / (# of CPUs)
    printf("Host load statistics\n");
    printf("  time period (sec) %5s%10s%10s\n", "5", "30", "60");
    printf("  load average %10u%10u%10u\n", load_info.avenrun[0],
           load_info.avenrun[1], load_info.avenrun[2]);
    printf("  Mach factor  %10u%10u%10u\n", load_info.mach_factor[0],
           load_info.mach_factor[1], load_info.mach_factor[2]);
   
    printf("\n");
   
    printf("Cumulative CPU load statistics\n");
    printf("  User state ticks     = %u\n",
           cpu_load_info.cpu_ticks[CPU_STATE_USER]);
    printf("  System state ticks   = %u\n",
           cpu_load_info.cpu_ticks[CPU_STATE_SYSTEM]);
    printf("  Nice state ticks     = %u\n",
           cpu_load_info.cpu_ticks[CPU_STATE_NICE]);
    printf("  Idle state ticks     = %u\n",
           cpu_load_info.cpu_ticks[CPU_STATE_IDLE]);
   
    printf("\n");
   
    printf("Virtual memory statistics\n");
    printf("  page size            = %u bytes\n", page_size);
    printf("  pages free           = %u\n", vm_stat.free_count);
    printf("  pages active         = %u\n", vm_stat.active_count);
    printf("  pages inactive       = %u\n", vm_stat.inactive_count);
    printf("  pages wired down     = %u\n", vm_stat.wire_count);
    printf("  zero fill pages      = %u\n", vm_stat.zero_fill_count);
    printf("  pages reactivated    = %u\n", vm_stat.reactivations);
    printf("  pageins              = %u\n", vm_stat.pageins);
    printf("  pageouts             = %u\n", vm_stat.pageouts);
    printf("  translation faults   = %u\n", vm_stat.faults);
    printf("  copy-on-write faults = %u\n", vm_stat.cow_faults);
    printf("  object cache lookups = %u\n", vm_stat.lookups);
    printf("  object cache hits    = %u (hit rate %2.2f %%)\n", vm_stat.hits,
           100 * (double)vm_stat.hits/(double)vm_stat.lookups);
   
    exit(0);
}
