// vm_rw_master.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
   
#define PROGNAME "vm_rw_master"
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
int
main(int argc, char **argv)
{
    kern_return_t          kr;
    pid_t                  pid;
    mach_port_t            target_task;
    mach_vm_address_t      address;
    mach_vm_size_t         size = (mach_vm_size_t)vm_page_size;
    vm_offset_t            local_address;
    mach_msg_type_number_t local_size = vm_page_size;
   
    if (argc != 3) {
        fprintf(stderr, "usage: %s <pid> <address in hex>\n", PROGNAME);
        exit(1);
    }
   
    pid = atoi(argv[1]);
    address = strtoul(argv[2], NULL, 16);
   
    kr = task_for_pid(mach_task_self(), pid, &target_task);
    EXIT_ON_MACH_ERROR("task_for_pid", kr);
   
    printf("reading address %p in target task\n", (void *)address);
   
    kr = mach_vm_read(target_task, address, size,  &local_address, &local_size);
    EXIT_ON_MACH_ERROR("vm_read", kr);
   
    // display some of the memory we read from the target task
    printf("read %u bytes from address %p in target task, first byte=%c\n",
           local_size, (void *)address, *(char *)local_address);
   
    // change some of the memory
    *(char *)local_address = 'B';
   
    // write it back to the target task
    kr = mach_vm_write(target_task, address, local_address, local_size);
    EXIT_ON_MACH_ERROR("vm_write", kr);
   
    exit(0);
}
