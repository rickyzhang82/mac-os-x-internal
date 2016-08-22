// kdebug.c
   
#define PROGNAME "kdebug"
   
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
   
struct proc;
   
// Kernel Debug definitions
#define PRIVATE
#define KERNEL_PRIVATE
#include <sys/kdebug.h>
#undef KERNEL_PRIVATE
#undef PRIVATE
   
// Configurable parameters
enum {
    KDBG_BSD_SYSTEM_CALL_OF_INTEREST = SYS_chdir,
    KDBG_SAMPLE_SIZE                 = 16384,
    KDBG_SAMPLE_INTERVAL             = 100000, // in microseconds
};
   
// Useful constants
enum {
    KDBG_FUNC_MASK   = 0xfffffffc, // for extracting function type
    KDBG_CLASS_MASK  = 0xff000000, // for extracting class type
    KDBG_CLASS_SHIFT = 24          // for extracting class type
};
   
// Global variables
int    exiting = 0; // avoid recursion in exit handlers
size_t oldlen;      // used while calling sysctl()
int    mib[8];      // used while calling sysctl()
pid_t  pid = -1;    // process ID of the traced process
   
// Global flags
int trace_enabled   = 0;
int set_remove_flag = 1;
   
// Mapping of kdebug class IDs to class names
const char *KDBG_CLASS_NAMES[256] = {
    NULL,           // 0
    "DBG_MACH",     // 1
    "DBG_NETWORK",  // 2
    "DBG_FSYSTEM",  // 3
    "DBG_BSD",      // 4
    "DBG_IOKIT",    // 5
    "DBG_DRIVERS",  // 6
    "DBG_TRACE",    // 7
    "DBG_DLIL",     // 8
    "DBG_SECURITY", // 9
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "DBG_MISC",     // 20
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "DBG_DYLD",     // 31
    "DBG_QT",       // 32
    "DBG_APPS",     // 33
    NULL,
};
   
// Functions that we implement (the 'u' in ukdbg represents user space)
void ukdbg_exit_handler(int);
void ukdbg_exit(const char *);
void ukdbg_setenable(int);
void ukdbg_clear();
void ukdbg_reinit();
void ukdbg_setbuf(int);
void ukdbg_getbuf(kbufinfo_t *);
void ukdbg_setpidcheck(pid_t, int);
void ukdbg_read(char *, size_t *);
void ukdbg_setreg_valcheck(int val1, int val2, int val3, int val4);
   
void
ukdbg_exit_handler(int s)
{
    exiting = 1;
   
    if (trace_enabled)
        ukdbg_setenable(0);
   
    if (pid > 0)
        ukdbg_setpidcheck(pid, 0);
   
    if (set_remove_flag)
        ukdbg_clear();
   
    fprintf(stderr, "cleaning up...\n");
   
    exit(s);
}
   
void
ukdbg_exit(const char *msg)
{
    if (msg)
        perror(msg);
   
    ukdbg_exit_handler(0);
}
   
// Enable or disable trace
// enable=1 enables (trace buffer must already be initialized)
// enable=0 disables
void
ukdbg_setenable(int enable)
{
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDENABLE;
    mib[3] = enable;
    if ((sysctl(mib, 4, NULL, &oldlen, NULL, 0) < 0) && !exiting)
        ukdbg_exit("ukdbg_setenable::sysctl");
   
    trace_enabled = enable;   
}
   
// Clean up relevant buffers
void
ukdbg_clear(void)
{
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDREMOVE;
    if ((sysctl(mib, 3, NULL, &oldlen, NULL, 0) < 0) && !exiting) {
        set_remove_flag = 0;
        ukdbg_exit("ukdbg_clear::sysctl");
    }
}
   
// Disable any ongoing trace collection and reinitialize the facility
void
ukdbg_reinit(void)
{
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDSETUP; 
    if (sysctl(mib, 3, NULL, &oldlen, NULL, 0) < 0)
        ukdbg_exit("ukdbg_reinit::sysctl");
}
   
// Set buffer for the desired number of trace entries
// Buffer size is limited to either 25% of physical memory (sane_size),
// or to the maximum mapped address, whichever is smaller
void
ukdbg_setbuf(int nbufs)
{
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDSETBUF;
    mib[3] = nbufs;
    if (sysctl(mib, 4, NULL, &oldlen, NULL, 0) < 0)
        ukdbg_exit("ukdbg_setbuf::sysctl");
}
   
// Turn pid check on or off in the trace buffer
// check=1 turns on pid check for this and all pids
// check=0 turns off pid check for this pid (but not all pids)
void
ukdbg_setpidcheck(pid_t pid, int check)
{
    kd_regtype kr;
    kr.type = KDBG_TYPENONE;
    kr.value1 = pid;
    kr.value2 = check;
    oldlen = sizeof(kd_regtype);
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDPIDTR;
    if ((sysctl(mib, 3, &kr, &oldlen, NULL, 0) < 0) && !exiting)
        ukdbg_exit("ukdbg_setpidcheck::sysctl");
}
   
// Set specific value checking
void
ukdbg_setreg_valcheck(int val1, int val2, int val3, int val4)
{
    kd_regtype kr;
    kr.type = KDBG_VALCHECK;
    kr.value1 = val1;
    kr.value2 = val2;
    kr.value3 = val3;
    kr.value4 = val4;
    oldlen = sizeof(kd_regtype);
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDSETREG;
    if (sysctl(mib, 3, &kr, &oldlen, NULL, 0) < 0)
        ukdbg_exit("ukdbg_setreg_valcheck::sysctl");
}
   
// Retrieve trace buffer information from the kernel
void
ukdbg_getbuf(kbufinfo_t *bufinfop)
{
    oldlen = sizeof(bufinfop);
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDGETBUF;         
    if (sysctl(mib, 3, bufinfop, &oldlen, 0, 0) < 0)
        ukdbg_exit("ukdbg_getbuf::sysctl");
}
   
// Retrieve some of the trace buffer from the kernel
void
ukdbg_read(char *buf, size_t *len)
{
    mib[0] = CTL_KERN;
    mib[1] = KERN_KDEBUG;
    mib[2] = KERN_KDREADTR;         
    if (sysctl(mib, 3, buf, len, NULL, 0) < 0)
        ukdbg_exit("ukdbg_read::sysctl");
}
   
int
main(int argc, char **argv)
{
    int            i, count;
    kd_buf        *kd;
    char          *kd_buf_memory;
    kbufinfo_t     bufinfo = { 0, 0, 0, 0 };
    unsigned short code;
   
    KDBG_CLASS_NAMES[255] = "DBG_MIG";
   
    if (argc > 2) {
        fprintf(stderr, "usage: %s [<pid>]\n", PROGNAME);
        exit(1);
    }
   
    if (argc == 2)
        pid = atoi(argv[1]);
   
    code = KDBG_BSD_SYSTEM_CALL_OF_INTEREST;
   
    // Arrange for cleanup
    signal(SIGHUP, ukdbg_exit_handler);
    signal(SIGINT, ukdbg_exit_handler);
    signal(SIGQUIT, ukdbg_exit_handler);
    signal(SIGTERM, ukdbg_exit_handler);
   
    kd_buf_memory = malloc(KDBG_SAMPLE_SIZE * sizeof(kd_buf));
    if (!kd_buf_memory) {
        perror("malloc");
        exit(1);
    }
   
    ukdbg_clear();                  // Clean up related buffers
    ukdbg_setbuf(KDBG_SAMPLE_SIZE); // Set buffer for the desired # of entries
    ukdbg_reinit();                 // Reinitialize the facility
    if (pid > 0)
        ukdbg_setpidcheck(pid, 1);  // We want this pid
    // We want this particular BSD system call
    ukdbg_setreg_valcheck(BSDDBG_CODE(DBG_BSD_EXCP_SC, code), 0, 0, 0);
    ukdbg_setenable(1);             // Enable tracing
   
    while (1) {
        ukdbg_getbuf(&bufinfo);                    // Query information
        oldlen = bufinfo.nkdbufs * sizeof(kd_buf); // How much to read?
        ukdbg_read(kd_buf_memory, &oldlen);        // Read that much
   
        count = oldlen;
   
        kd = (kd_buf *)kd_buf_memory;
        for (i = 0; i < count; i++) {
   
            char     *qual = "";
            uint64_t  cpu, now;
            int       debugid, thread, type, class;
   
            thread = kd[i].arg5;
            debugid = kd[i].debugid;
            type = debugid & KDBG_FUNC_MASK;
            class = (debugid & KDBG_CLASS_MASK) >> KDBG_CLASS_SHIFT;
            now = kd[i].timestamp & KDBG_TIMESTAMP_MASK;
            cpu = (kd[i].timestamp & KDBG_CPU_MASK) >> KDBG_CPU_SHIFT;
   
            if (debugid & DBG_FUNC_START)
                qual = "DBG_FUNC_START";
            else if (debugid & DBG_FUNC_END)
                qual = "DBG_FUNC_END";
   
            // Note that 'type' should be the system call we were looking for
            // (type == BSDDBG_CODE(DBG_BSD_EXCP_SC, code) is true
   
            printf("%lld: cpu %lld %s code %#x thread %p %s\n",
                   now,
                   cpu,
                   (KDBG_CLASS_NAMES[class]) ? KDBG_CLASS_NAMES[class] : "",
                   type,
                   (void *)thread,
                   qual);
        }
   
        usleep(KDBG_SAMPLE_INTERVAL);
    }
}
