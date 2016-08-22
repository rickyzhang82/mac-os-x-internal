// lsports.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
#define PROGNAME "lsports"
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
void
print_mach_port_type(mach_port_type_t type)
{
    if (type & MACH_PORT_TYPE_SEND)      { printf("SEND ");      }
    if (type & MACH_PORT_TYPE_RECEIVE)   { printf("RECEIVE ");   }
    if (type & MACH_PORT_TYPE_SEND_ONCE) { printf("SEND_ONCE "); }
    if (type & MACH_PORT_TYPE_PORT_SET)  { printf("PORT_SET ");  }
    if (type & MACH_PORT_TYPE_DEAD_NAME) { printf("DEAD_NAME "); }
    if (type & MACH_PORT_TYPE_DNREQUEST) { printf("DNREQUEST "); }
    printf("\n");
}
   
int
main(int argc, char **argv)
{
    int                    i;
    pid_t                  pid;
    kern_return_t          kr;
    mach_port_name_array_t names;
    mach_port_type_array_t types;
    mach_msg_type_number_t ncount, tcount;
    mach_port_limits_t     port_limits;
    mach_port_status_t     port_status;
    mach_msg_type_number_t port_info_count;
    task_t                 task;
    task_t                 mytask = mach_task_self();
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <pid>\n", PROGNAME);
        exit(1);
    }
   
    pid = atoi(argv[1]);
    kr = task_for_pid(mytask, (int)pid, &task);
    EXIT_ON_MACH_ERROR("task_for_pid", kr);
 
    // retrieve a list of the rights present in the given task's IPC space,
    // along with type information (no particular ordering)
    kr = mach_port_names(task, &names, &ncount, &types, &tcount);
    EXIT_ON_MACH_ERROR("mach_port_names", kr);
   
    printf("%8s %8s %8s %8s %8s task rights\n",
           "name", "q-limit", "seqno", "msgcount", "sorights");
    for (i = 0; i < ncount; i++) {
        printf("%08x ", names[i]);
   
        // get resource limits for the port
        port_info_count = MACH_PORT_LIMITS_INFO_COUNT;
        kr = mach_port_get_attributes(
                 task,                           // the IPC space in question
                 names[i],                       // task's name for the port
                 MACH_PORT_LIMITS_INFO,          // information flavor desired
                 (mach_port_info_t)&port_limits, // outcoming information
                 &port_info_count);              // size returned
        if (kr == KERN_SUCCESS)
            printf("%8d ", port_limits.mpl_qlimit);
        else
            printf("%8s ", "-");
   
        // get miscellaneous information about associated rights and messages
        port_info_count = MACH_PORT_RECEIVE_STATUS_COUNT;
        kr = mach_port_get_attributes(task, names[i], MACH_PORT_RECEIVE_STATUS,
                                      (mach_port_info_t)&port_status,
                                      &port_info_count);
        if (kr == KERN_SUCCESS) {
            printf("%8d %8d %8d ",
                   port_status.mps_seqno,     // current sequence # for the port
                   port_status.mps_msgcount,  // # of messages currently queued
                   port_status.mps_sorights); // # of send-once rights
        } else
            printf("%8s %8s %8s ", "-", "-", "-");
        print_mach_port_type(types[i]);
    }
   
    vm_deallocate(mytask, (vm_address_t)names, ncount*sizeof(mach_port_name_t));
    vm_deallocate(mytask, (vm_address_t)types, tcount*sizeof(mach_port_type_t));
   
    exit(0);
}
