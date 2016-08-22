// processor_info.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
void
print_basic_info(processor_basic_info_t info)
{
    printf("CPU: slot %d%s %s, type %d, subtype %d\n", info->slot_num,
           (info->is_master) ? " (master)," : ",",
           (info->running) ? "running" : "not running",
           info->cpu_type, info->cpu_subtype);
}
   
void
print_cpu_load_info(processor_cpu_load_info_t info)
{
    unsigned long ticks;
   
    // Total ticks do not amount to the uptime if the machine has slept
    ticks = info->cpu_ticks[CPU_STATE_USER]   +
            info->cpu_ticks[CPU_STATE_SYSTEM] +
            info->cpu_ticks[CPU_STATE_IDLE]   +
            info->cpu_ticks[CPU_STATE_NICE];
    printf("     %ld ticks "
           "(user %ld, system %ld, idle %ld, nice %ld)\n", ticks,
           info->cpu_ticks[CPU_STATE_USER],
           info->cpu_ticks[CPU_STATE_SYSTEM],
           info->cpu_ticks[CPU_STATE_IDLE],
           info->cpu_ticks[CPU_STATE_NICE]);
    printf("     cpu uptime %ld h %ld m %ld s\n",
          (ticks / 100) / 3600,        // hours
          ((ticks / 100) % 3600) / 60, // minutes
          (ticks / 100) % 60);         // seconds
}
   
int
main(void)
{
    int                            i;
    kern_return_t                  kr;
    host_name_port_t               myhost;
    host_priv_t                    host_priv;
    processor_port_array_t         processor_list;
    natural_t                      processor_count;
    processor_basic_info_data_t    basic_info;
    processor_cpu_load_info_data_t cpu_load_info;
    natural_t                      info_count;
   
    myhost = mach_host_self();
    kr = host_get_host_priv_port(myhost, &host_priv);
    if (kr != KERN_SUCCESS) {
        mach_error("host_get_host_priv_port:", kr);
        exit(1);
    }
   
    kr = host_processors(host_priv, &processor_list, &processor_count);
    if (kr != KERN_SUCCESS) {
        mach_error("host_processors:", kr);
        exit(1);
    }
   
    printf("%d processors total.\n", processor_count);
   
    for (i = 0; i < processor_count; i++) {
        info_count = PROCESSOR_BASIC_INFO_COUNT;
        kr = processor_info(processor_list[i],
                            PROCESSOR_BASIC_INFO,
                            &myhost, 
                            (processor_info_t)&basic_info,
                            &info_count);
        if (kr == KERN_SUCCESS)
            print_basic_info((processor_basic_info_t)&basic_info);
   
        info_count = PROCESSOR_CPU_LOAD_INFO_COUNT;
        kr = processor_info(processor_list[i],
                            PROCESSOR_CPU_LOAD_INFO,
                            &myhost, 
                            (processor_info_t)&cpu_load_info,
                            &info_count);
        if (kr == KERN_SUCCESS)
            print_cpu_load_info((processor_cpu_load_info_t)&cpu_load_info);
    }
   
    // Other processor information flavors (may be unsupported)
    //
    //  PROCESSOR_PM_REGS_INFO,  // performance monitor register information
    //  PROCESSOR_TEMPERATURE,   // core temperature
   
   
    // This will deallocate while rounding up to page size
    (void)vm_deallocate(mach_task_self(), (vm_address_t)processor_list,
                        processor_count * sizeof(processor_t *));
   
    exit(0);
}
