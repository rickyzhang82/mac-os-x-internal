// task_create.c
   
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
   
int
main(int argc, char **argv)
{
    kern_return_t           kr;
    pid_t                   pid;
    task_t                  child_task;
    ledger_t                ledgers;
    ledger_array_t          ledger_array;
    mach_msg_type_number_t  ledger_count;
    boolean_t               inherit = TRUE;
    task_info_data_t        info;
    mach_msg_type_number_t  count;
    struct task_basic_info *task_basic_info;
   
    if (argc == 2)
        inherit = (atoi(argv[1])) ? TRUE : FALSE;
   
    // have the kernel use the parent task's ledger
    ledger_count = 1;
    ledgers = (ledger_t)0;
    ledger_array = &ledgers;
   
    // create the new task
    kr = task_create(mach_task_self(), // prototype (parent) task
                     ledger_array,     // resource ledgers
                     ledger_count,     // number of ledger ports
                     inherit,          // inherit memory?
                     &child_task);     // port for new task
    if (kr != KERN_SUCCESS) {
        mach_error("task_create:", kr);
        exit(1);
    }
   
    // get information on the new task
    count = TASK_INFO_MAX;
    kr = task_info(child_task, TASK_BASIC_INFO, (task_info_t)info, &count);
    if (kr != KERN_SUCCESS)
        mach_error("task_info:", kr);
    else {
        // there should be no BSD process ID
        kr = pid_for_task(child_task, &pid);
        if (kr != KERN_SUCCESS)
            mach_error("pid_for_task:", kr);
   
        task_basic_info = (struct task_basic_info *)info;
        printf("pid %d, virtual sz %d KB, resident sz %d KB\n", pid,
                task_basic_info->virtual_size >> 10,
                task_basic_info->resident_size >> 10);
    }
   
    kr = task_terminate(child_task);
    if (kr != KERN_SUCCESS)
        mach_error("task_terminate:", kr);
   
    exit(0);
}
