// lstasks.c
   
#include <getopt.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <Carbon/Carbon.h>
   
#define PROGNAME  "lstasks"
   
// pretty-printing macros
#define INDENT_L1 "  "
#define INDENT_L2 "    "
#define INDENT_L3 "      "
#define INDENT_L4 "        "
#define SUMMARY_HEADER \
    "task#   BSD pid program         PSN (high)   PSN (low)    #threads\n"
   
static const char *task_roles[] = {
    "RENICED",
    "UNSPECIFIED",
    "FOREGROUND_APPLICATION",
    "BACKGROUND_APPLICATION",
    "CONTROL_APPLICATION",
    "GRAPHICS_SERVER",
};
#define TASK_ROLES_MAX (sizeof(task_roles)/sizeof(char *))
   
static const char *thread_policies[] = {
    "UNKNOWN?",
    "STANDARD|EXTENDED",
    "TIME_CONSTRAINT",
    "PRECEDENCE",
};
#define THREAD_POLICIES_MAX (sizeof(thread_policies)/sizeof(char *))
   
static const char *thread_states[] = {
    "NONE",
    "RUNNING",
    "STOPPED",
    "WAITING",
    "UNINTERRUPTIBLE",
    "HALTED",
};
#define THREAD_STATES_MAX (sizeof(thread_states)/sizeof(char *))
   
#define EXIT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); exit((retval)); }
   
// get BSD process name from process ID
static char *
getprocname(pid_t pid)
{
    size_t len = sizeof(struct kinfo_proc);
    static int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, 0 };
    static struct kinfo_proc kp;
   
    name[3] = pid;
    kp.kp_proc.p_comm[0] = '\0';
   
    if (sysctl((int *)name, sizeof(name)/sizeof(*name), &kp, &len, NULL, 0))
        return "?";
   
    if (kp.kp_proc.p_comm[0] == '\0')
        return "exited?";
   
    return kp.kp_proc.p_comm;
}
   
void
usage()
{
    printf("usage: %s [-s|-v] [-p <pid>]\n", PROGNAME);
    exit(1);
}
   
// used as the printf() while printing only the summary
int
noprintf(const char *format, ...)
{
    return 0; // nothing
}
   
int
main(int argc, char **argv)
{
    int i, j, summary = 0, verbose = 0;
    int (* Printf)(const char *format, ...);
   
    pid_t pid;
   
    // for Carbon processes
    OSStatus            status;
    ProcessSerialNumber psn;
    CFStringRef         nameRef;
    char                name[MAXPATHLEN];
   
    kern_return_t kr;
    mach_port_t   myhost;                // host port
    mach_port_t   mytask;                // our task
    mach_port_t   onetask = 0;           // we want only one specific task
    mach_port_t   p_default_set;         // processor set name port
    mach_port_t   p_default_set_control; // processor set control port
   
    //// for task-related querying
   
    // pointer to ool buffer for processor_set_tasks(), and the size of
    // the data actually returned in the ool buffer
    task_array_t           task_list;
    mach_msg_type_number_t task_count;
   
    // maximum-sized buffer for task_info(), and the size of the data
    // actually filled in by task_info()
    task_info_data_t       tinfo;
    mach_msg_type_number_t task_info_count;
   
    // flavor-specific pointers to cast the generic tinfo buffer
    task_basic_info_t        basic_info;
    task_events_info_t       events_info;
    task_thread_times_info_t thread_times_info;
    task_absolutetime_info_t absolutetime_info;
   
    // used for calling task_get_policy()
    task_category_policy_data_t category_policy;
    boolean_t get_default;
   
    // opaque token that identifies the task as a BSM audit subject
    audit_token_t    audit_token;
    security_token_t security_token; // kernel's security token is { 0, 1 }
   
    //// for thread-related querying
   
    // pointer to ool buffer for task_threads(), and the size of the data
    // actually returned in the ool buffer
    thread_array_t         thread_list;
    mach_msg_type_number_t thread_count;
   
    // maximum-sized buffer for thread_info(), and the size of the data
    // actually filled in by thread_info()
    thread_info_data_t     thinfo;
    mach_msg_type_number_t thread_info_count;
   
    // flavor-specific pointers to cast the generic thinfo buffer
    thread_basic_info_t basic_info_th;
   
    // used for calling thread_get_policy()
    thread_extended_policy_data_t        extended_policy;
    thread_time_constraint_policy_data_t time_constraint_policy;
    thread_precedence_policy_data_t      precedence_policy;
   
    // to count individual types of process subsystem entities
    uint32_t stat_task = 0;   // Mach tasks
    uint32_t stat_proc = 0;   // BSD processes
    uint32_t stat_cpm = 0;    // Carbon Process Manager processes
    uint32_t stat_thread = 0; // Mach threads
   
    // assume we won't be silent: use the verbose version of printf() by default
    Printf = printf;
   
    myhost = mach_host_self();
    mytask = mach_task_self();
   
    while ((i = getopt(argc, argv, "p:sv")) != -1) {
        switch (i) {
            case 'p':
                pid = strtoul(optarg, NULL, 10);
                kr = task_for_pid(mytask, pid, &onetask);
                EXIT_ON_MACH_ERROR("task_for_pid", 1);
                break;
            case 's':
                summary = 1;
                Printf = noprintf;
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                usage();
        }
    }
   
    // can't have both
    if (summary && verbose)
        usage();
   
    argv += optind;
    argc -= optind;
   
    kr = processor_set_default(myhost, &p_default_set);
    EXIT_ON_MACH_ERROR("processor_default", 1);
   
    // get the privileged port so that we can get all tasks
    kr = host_processor_set_priv(myhost, p_default_set, &p_default_set_control);
    EXIT_ON_MACH_ERROR("host_processor_set_priv", 1);
   
    // we could check for multiple processor sets, but we know there aren't...
    kr = processor_set_tasks(p_default_set_control, &task_list, &task_count);
    EXIT_ON_MACH_ERROR("processor_set_tasks", 1);
   
    if (!verbose)
        Printf(SUMMARY_HEADER);
   
    // check out each task
    for (i = 0; i < task_count; i++) {
   
        // ignore our own task
        if (task_list[i] == mytask)
            continue;
   
        if (onetask && (task_list[i] != onetask))
            continue;
   
        pid = 0;
        status = procNotFound;
   
        // note that we didn't count this task
        stat_task++;
   
        if (verbose)
            Printf("Task #%d\n", i);
        else
            Printf("%5d", i);
   
        // check for BSD process (process 0 not counted as a BSD process)
        kr = pid_for_task(task_list[i], &pid);
        if ((kr == KERN_SUCCESS) && (pid > 0)) {
            stat_proc++;
   
            if (verbose)
                Printf(INDENT_L1 "BSD process id (pid)   = %u (%s)\n", pid,
                       getprocname(pid));
            else
                Printf("    %6u %-16s", pid, getprocname(pid));
         } else // no BSD process
            if (verbose)
                Printf(INDENT_L1 "BSD process id (pid)   = "
                       "/* not a BSD process */\n");
            else
                Printf("    %6s %-16s", "-", "-");
   
        // check whether there is a process serial number
        if (pid > 0)
            status = GetProcessForPID(pid, &psn);
        if (status == noErr) {
            stat_cpm++;
            if (verbose) {
                status = CopyProcessName(&psn, &nameRef);
                CFStringGetCString(nameRef, name, MAXPATHLEN,
                                   kCFStringEncodingASCII);
                Printf(INDENT_L1 "Carbon process name    = %s\n", name);
                CFRelease(nameRef);
            } else
                Printf(" %-12d%-12d", psn.highLongOfPSN, psn.lowLongOfPSN);
        } else // no PSN
            if (verbose)
                Printf(INDENT_L1 "Carbon process name    = "
                       "/* not a Carbon process */\n");
            else
                Printf(" %-12s%-12s", "-", "-");
   
        if (!verbose)
            goto do_threads;
   
        // basic task information
        task_info_count = TASK_INFO_MAX;
        kr = task_info(task_list[i], TASK_BASIC_INFO, (task_info_t)tinfo,
                       &task_info_count);
        if (kr != KERN_SUCCESS) {
            mach_error("task_info:", kr);
            fprintf(stderr, "*** TASK_BASIC_INFO failed (task=%x)\n",
                    task_list[i]);
            // skip this task
            continue;
        }
        basic_info = (task_basic_info_t)tinfo;
        Printf(INDENT_L2 "virtual size         = %u KB\n",
               basic_info->virtual_size >> 10);
        Printf(INDENT_L2 "resident size        = %u KB\n",
               basic_info->resident_size >> 10);
        if ((basic_info->policy < 0) &&
            (basic_info->policy > THREAD_POLICIES_MAX))
            basic_info->policy = 0;
        Printf(INDENT_L2 "default policy       = %u (%s)\n",
               basic_info->policy, thread_policies[basic_info->policy]);
   
        Printf(INDENT_L1 "Thread run times\n");
   
        Printf(INDENT_L2 "user (terminated)    = %u s %u us\n",
               basic_info->user_time.seconds,
               basic_info->user_time.microseconds);
        Printf(INDENT_L2 "system (terminated)  = %u s %u us\n",
               basic_info->system_time.seconds,
               basic_info->system_time.microseconds);
   
        // times for live threads (unreliable -- we are not suspending)
        task_info_count = TASK_INFO_MAX;
        kr = task_info(task_list[i], TASK_THREAD_TIMES_INFO,
                       (task_info_t)tinfo, &task_info_count);
        if (kr == KERN_SUCCESS) {
            thread_times_info = (task_thread_times_info_t)tinfo;
            Printf(INDENT_L2 "user (live)          = %u s %u us\n",
                   thread_times_info->user_time.seconds,
                   thread_times_info->user_time.microseconds);
            Printf(INDENT_L2 "system (live)        = %u s %u us\n",
                   thread_times_info->system_time.seconds,
                   thread_times_info->system_time.microseconds);
        }
   
        // absolute times for live threads, and overall absolute time
        task_info_count = TASK_INFO_MAX;
        kr = task_info(task_list[i], TASK_ABSOLUTETIME_INFO,
                       (task_info_t)tinfo, &task_info_count);
        if (kr == KERN_SUCCESS) {
            Printf(INDENT_L1 "Thread times (absolute)\n");
            absolutetime_info = (task_absolutetime_info_t)tinfo;
            Printf(INDENT_L2 "user (total)         = %lld\n",
                   absolutetime_info->total_user);
            Printf(INDENT_L2 "system (total)       = %lld\n",
                   absolutetime_info->total_system);
            Printf(INDENT_L2 "user (live)          = %lld\n",
                   absolutetime_info->threads_user);
            Printf(INDENT_L2 "system (live)        = %lld\n",
                   absolutetime_info->threads_system);
        }
   
        // events
        task_info_count = TASK_INFO_MAX;
        kr = task_info(task_list[i], TASK_EVENTS_INFO, (task_info_t)tinfo,
                       &task_info_count);
        if (kr == KERN_SUCCESS) {
            events_info = (task_events_info_t)tinfo;
            Printf(INDENT_L2 "page faults          = %u\n",
                    events_info->faults); 
            Printf(INDENT_L2 "actual pageins       = %u\n",
                   events_info->pageins);
            Printf(INDENT_L2 "copy-on-write faults = %u\n",
                   events_info->cow_faults);
            Printf(INDENT_L2 "messages sent        = %u\n",
                   events_info->messages_sent);
            Printf(INDENT_L2 "messages received    = %u\n",
                   events_info->messages_received);
            Printf(INDENT_L2 "Mach system calls    = %u\n",
                   events_info->syscalls_mach);
            Printf(INDENT_L2 "Unix system calls    = %u\n",
                   events_info->syscalls_unix);
            Printf(INDENT_L2 "context switches     = %u\n",
                   events_info->csw);
        }
   
        // task policy information
        task_info_count = TASK_CATEGORY_POLICY_COUNT;
        get_default = FALSE;
        kr = task_policy_get(task_list[i], TASK_CATEGORY_POLICY,
                             (task_policy_t)&category_policy,
                             &task_info_count, &get_default);
        if (kr == KERN_SUCCESS) {
            if (get_default == FALSE) {
                if ((category_policy.role >= -1) &&
                    (category_policy.role < (TASK_ROLES_MAX - 1)))
                    Printf(INDENT_L2 "role                 = %s\n",
                           task_roles[category_policy.role + 1]);
            } else // no current settings -- other parameters take precedence
                Printf(INDENT_L2 "role                 = NONE\n");
        }
   
        // audit token
        task_info_count = TASK_AUDIT_TOKEN_COUNT;
        kr = task_info(task_list[i], TASK_AUDIT_TOKEN,
                       (task_info_t)&audit_token, &task_info_count);
        if (kr == KERN_SUCCESS) {
            int n;
            Printf(INDENT_L2 "audit token          = ");
            for (n = 0; n < sizeof(audit_token)/sizeof(uint32_t); n++)
                Printf("%x ", audit_token.val[n]);
            Printf("\n");
        }
   
        // security token
        task_info_count = TASK_SECURITY_TOKEN_COUNT;
        kr = task_info(task_list[i], TASK_SECURITY_TOKEN,
                       (task_info_t)&security_token, &task_info_count);
        if (kr == KERN_SUCCESS) {
            int n;
            Printf(INDENT_L2 "security token       = ");
            for (n = 0; n < sizeof(security_token)/sizeof(uint32_t); n++)
                Printf("%x ", security_token.val[n]);
            Printf("\n");
        }
   
do_threads:
   
        // get threads in the task
        kr = task_threads(task_list[i], &thread_list, &thread_count);
        if (kr != KERN_SUCCESS) {
            mach_error("task_threads:", kr);
            fprintf(stderr, "task_threads() failed (task=%x)\n", task_list[i]);
            continue;
        }
   
        if (thread_count > 0)
            stat_thread += thread_count;
   
        if (!verbose) {
            Printf(" %8d\n", thread_count);
            continue;
        }
   
        Printf(INDENT_L1 "Threads in this task   = %u\n", thread_count);
   
        // check out threads
        for (j = 0; j < thread_count; j++) {
   
            thread_info_count = THREAD_INFO_MAX;
            kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
                             (thread_info_t)thinfo, &thread_info_count);
            if (kr != KERN_SUCCESS) {
                mach_error("task_info:", kr);
                fprintf(stderr,
                        "*** thread_info() failed (task=%x thread=%x)\n",
                        task_list[i], thread_list[j]);
                continue;
            }
   
            basic_info_th = (thread_basic_info_t)thinfo;
            Printf(INDENT_L2 "thread %u/%u (%p) in task %u (%p)\n",
                   j, thread_count - 1, thread_list[j], i, task_list[i]);
   
            Printf(INDENT_L3 "user run time                = %u s %u us\n",
                   basic_info_th->user_time.seconds,
                   basic_info_th->user_time.microseconds);
            Printf(INDENT_L3 "system run time              = %u s %u us\n",
                   basic_info_th->system_time.seconds,
                   basic_info_th->system_time.microseconds);
            Printf(INDENT_L3 "scaled cpu usage percentage  = %u\n",
                   basic_info_th->cpu_usage);
   
            switch (basic_info_th->policy) {
   
            case THREAD_EXTENDED_POLICY:
                get_default = FALSE;
                thread_info_count = THREAD_EXTENDED_POLICY_COUNT;
                kr = thread_policy_get(thread_list[j], THREAD_EXTENDED_POLICY,
                                       (thread_policy_t)&extended_policy,
                                       &thread_info_count, &get_default);
                if (kr != KERN_SUCCESS)
                    break;
                Printf(INDENT_L3 "scheduling policy            = %s\n",
                       (extended_policy.timeshare == TRUE) ? \
                           "STANDARD" : "EXTENDED");
                break;
   
            case THREAD_TIME_CONSTRAINT_POLICY:
                get_default = FALSE;
                thread_info_count = THREAD_TIME_CONSTRAINT_POLICY_COUNT;
                kr = thread_policy_get(thread_list[j],
                                       THREAD_TIME_CONSTRAINT_POLICY,
                                       (thread_policy_t)&time_constraint_policy,
                                       &thread_info_count, &get_default);
                if (kr != KERN_SUCCESS)
                    break;
                Printf(INDENT_L3 "scheduling policy            = " \
                       "TIME_CONSTRAINT\n");
                Printf(INDENT_L4   "period                     = %-4u\n",
                       time_constraint_policy.period);
                Printf(INDENT_L4   "computation                = %-4u\n",
                       time_constraint_policy.computation);
                Printf(INDENT_L4   "constraint                 = %-4u\n",
                       time_constraint_policy.constraint);
                Printf(INDENT_L4   "preemptible                = %s\n",
                       (time_constraint_policy.preemptible == TRUE) ? \
                           "TRUE" : "FALSE");
                break;
   
            case THREAD_PRECEDENCE_POLICY:
                get_default = FALSE;
                thread_info_count = THREAD_PRECEDENCE_POLICY;
                kr = thread_policy_get(thread_list[j], THREAD_PRECEDENCE_POLICY,
                                       (thread_policy_t)&precedence_policy,
                                       &thread_info_count, &get_default);
                if (kr != KERN_SUCCESS)
                    break;
                Printf(INDENT_L3 "scheduling policy            = PRECEDENCE\n");
                Printf(INDENT_L4 "importance                 = %-4u\n",
                       precedence_policy.importance);
                break;
   
            default:
                Printf(INDENT_L3 "scheduling policy            = UNKNOWN?\n");
                break;
            }
   
            Printf(INDENT_L3
                   "run state                    = %-4u (%s)\n",
                   basic_info_th->run_state,
                   (basic_info_th->run_state >= THREAD_STATES_MAX) ? \
                       "?" : thread_states[basic_info_th->run_state]);
   
            Printf(INDENT_L3
                   "flags                        = %-4x%s",
                   basic_info_th->flags,
                   (basic_info_th->flags & TH_FLAGS_IDLE) ? \
                       " (IDLE)\n" : "\n");
   
            Printf(INDENT_L3 "suspend count                = %u\n",
                   basic_info_th->suspend_count);
            Printf(INDENT_L3 "sleeping for time            = %u s\n",
                   basic_info_th->sleep_time);
   
        } // for each thread
   
        vm_deallocate(mytask, (vm_address_t)thread_list,
                      thread_count * sizeof(thread_act_t));
   
    } // for each task
   
    Printf("\n");
   
    fprintf(stdout, "%4d Mach tasks\n%4d Mach threads\n"
            "%4d BSD processes\n%4d CPM processes\n",
            stat_task, stat_thread, stat_proc, stat_cpm);
   
    vm_deallocate(mytask, (vm_address_t)task_list, task_count * sizeof(task_t));
   
    exit(0);
}
