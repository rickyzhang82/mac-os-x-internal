// host_basic_info.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
int
main()
{
    kern_return_t           kr; // the standard return type for Mach calls
    host_name_port_t        myhost;
    kernel_version_t        kversion;
    host_basic_info_data_t  hinfo;
    mach_msg_type_number_t  count;
    char                   *cpu_type_name, *cpu_subtype_name;
    vm_size_t               page_size;
    
    // get send rights to the name port for the current host
    myhost = mach_host_self();
   
    kr = host_kernel_version(myhost, kversion);
    EXIT_ON_MACH_ERROR("host_kernel_version", kr);
   
    count = HOST_BASIC_INFO_COUNT;      // size of the buffer
    kr = host_info(myhost,              // the host name port
                   HOST_BASIC_INFO,     // flavor
                   (host_info_t)&hinfo, // out structure
                   &count);             // in/out size
    EXIT_ON_MACH_ERROR("host_info", kr);
   
    kr = host_page_size(myhost, &page_size);
    EXIT_ON_MACH_ERROR("host_page_size", kr);
   
    printf("%s\n", kversion);
   
    // the slot_name() library function converts the specified
    // cpu_type/cpu_subtype pair to a human-readable form
    slot_name(hinfo.cpu_type, hinfo.cpu_subtype, &cpu_type_name,
              &cpu_subtype_name);
   
    printf("cpu              %s (%s, type=0x%x subtype=0x%x "
           "threadtype=0x%x)\n", cpu_type_name, cpu_subtype_name,
           hinfo.cpu_type, hinfo.cpu_subtype, hinfo.cpu_threadtype);
    printf("max_cpus         %d\n", hinfo.max_cpus);
    printf("avail_cpus       %d\n", hinfo.avail_cpus);
    printf("physical_cpu     %d\n", hinfo.physical_cpu);
    printf("physical_cpu_max %d\n", hinfo.physical_cpu_max);
    printf("logical_cpu      %d\n", hinfo.logical_cpu);
    printf("logical_cpu_max  %d\n", hinfo.logical_cpu_max);
    printf("memory_size      %u MB\n", (hinfo.memory_size >> 20));
    printf("max_mem          %llu MB\n", (hinfo.max_mem >> 20));
    printf("page_size        %u bytes\n", page_size);
   
    exit(0);
}
