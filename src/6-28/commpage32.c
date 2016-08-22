// commpage32.c
   
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
   
#define PRIVATE
#define KERNEL_PRIVATE

#include <machine/cpu_capabilities.h>
#include <machine/commpage.h>
   
#define WSPACE_FMT_SZ "24"
#define WSPACE_FMT "%-" WSPACE_FMT_SZ "s = "
   
#define CP_CAST_TO_U_INT32(x)  (u_int32_t)(*(u_int32_t *)(x))
#define ADDR2DESC(x)           (commpage_descriptor *)&(CP_CAST_TO_U_INT32(x))
   
#define CP_PRINT_U_INT8_BOOL(label, item) \
    printf(WSPACE_FMT "%s\n", label, \
        ((u_int8_t)(*(u_int8_t *)(item))) ? "yes" : "no")
#define CP_PRINT_U_INT16(label, item) \
    printf(WSPACE_FMT "%hd\n", label, (u_int16_t)(*(u_int16_t *)(item)))
#define CP_PRINT_U_INT32(label, item) \
    printf(WSPACE_FMT "%u\n", label, (u_int32_t)(*(u_int32_t *)(item)))
#define CP_PRINT_U_INT64(label, item) \
    printf(WSPACE_FMT "%#llx\n", label, (u_int64_t)(*(u_int64_t *)(item)))
#define CP_PRINT_D_FLOAT(label, item) \
    printf(WSPACE_FMT "%lf\n", label, (double)(*(double *)(item)))
   
const char *
cpuCapStrings[] = {
#if defined (__ppc__)
    "kHasAltivec",             // << 0
    "k64Bit",                  // << 1
    "kCache32",                // << 2
    "kCache64",                // << 3
    "kCache128",               // << 4
    "kDcbaRecommended",        // << 5
    "kDcbaAvailable",          // << 6
    "kDataStreamsRecommended", // << 7
    "kDataStreamsAvailable",   // << 8
    "kDcbtStreamsRecommended", // << 9
    "kDcbtStreamsAvailable",   // << 10
    "kFastThreadLocalStorage", // << 11
#else /* __i386__ */
    "kHasMMX",                 // << 0
    "kHasSSE",                 // << 1
    "kHasSSE2",                // << 2
    "kHasSSE3",                // << 3
    "kCache32",                // << 4
    "kCache64",                // << 5
    "kCache128",               // << 6
    "kFastThreadLocalStorage", // << 7
    "NULL",                    // << 8
    "NULL",                    // << 9
    "NULL",                    // << 10
    "NULL",                    // << 11
#endif
    NULL,                      // << 12
    NULL,                      // << 13
    NULL,                      // << 14
    "kUP",                     // << 15
    NULL,                      // << 16
    NULL,                      // << 17
    NULL,                      // << 18
    NULL,                      // << 19
    NULL,                      // << 20
    NULL,                      // << 21
    NULL,                      // << 22
    NULL,                      // << 23
    NULL,                      // << 24
    NULL,                      // << 25
    NULL,                      // << 26
    "kHasGraphicsOps",         // << 27
    "kHasStfiwx",              // << 28
    "kHasFsqrt",               // << 29
    NULL,                      // << 30
    NULL,                      // << 31
};
   
void print_bits32(u_int32_t);
void print_cpu_capabilities(u_int32_t);
void print_commpage_descriptor(const char *, u_int32_t);
   
void
print_bits32(u_int32_t u)
{
    u_int32_t i;
   
    for (i = 32; i--; putchar(u & 1 << i ? '1' : '0')); 
}
   
void
print_cpu_capabilities(u_int32_t cap)
{
    int i;
    printf(WSPACE_FMT, "cpu capabilities (bits)");
    print_bits32(cap);
    printf("\n");
    for (i = 0; i < 31; i++)
        if (cpuCapStrings[i] && (cap & (1 << i)))
            printf("%-" WSPACE_FMT_SZ "s   + %s\n", " ", cpuCapStrings[i]);
}
   
void
print_commpage_descriptor(const char *label, u_int32_t addr)
{
    commpage_descriptor *d = ADDR2DESC(addr);
    printf("%s @ %08x\n", label, addr);
#if defined (__ppc__)
    printf("  code_offset      = %hd\n", d->code_offset);
    printf("  code_length      = %hd\n", d->code_length);
    printf("  commpage_address = %hx\n", d->commpage_address);
    printf("  special          = %#hx\n", d->special);
#else /* __i386__ */
    printf("  code_address     = %p\n", d->code_address);
    printf("  code_length      = %ld\n", d->code_length);
    printf("  commpage_address = %#lx\n", d->commpage_address);
#endif
    printf("  musthave         = %#lx\n", d->musthave);
    printf("  canthave         = %#lx\n", d->canthave);
}
   
int
main(void)
{
    u_int32_t u;
   
    printf(WSPACE_FMT "%#08x\n", "base address", _COMM_PAGE_BASE_ADDRESS);
    printf(WSPACE_FMT "%s\n", "signature", (char *)_COMM_PAGE_BASE_ADDRESS);
    CP_PRINT_U_INT16("version", _COMM_PAGE_VERSION);
   
    u = CP_CAST_TO_U_INT32(_COMM_PAGE_CPU_CAPABILITIES);
    printf(WSPACE_FMT "%u\n", "number of processors",
          (u & kNumCPUs) >> kNumCPUsShift);
    print_cpu_capabilities(u);
    CP_PRINT_U_INT16("cache line size", _COMM_PAGE_CACHE_LINESIZE);
#if defined (__ppc__)
    CP_PRINT_U_INT8_BOOL("AltiVec available?", _COMM_PAGE_ALTIVEC);
    CP_PRINT_U_INT8_BOOL("64-bit processor?", _COMM_PAGE_64_BIT);
#endif
    CP_PRINT_D_FLOAT("two52 (2^52)", _COMM_PAGE_2_TO_52);
    CP_PRINT_D_FLOAT("ten6 (10^6)", _COMM_PAGE_10_TO_6);
    CP_PRINT_U_INT64("timebase", _COMM_PAGE_TIMEBASE);
    CP_PRINT_U_INT32("timestamp (s)", _COMM_PAGE_TIMESTAMP);
    CP_PRINT_U_INT32("timestamp (us)", _COMM_PAGE_TIMESTAMP + 0x04);
    CP_PRINT_U_INT64("seconds per tick", _COMM_PAGE_SEC_PER_TICK);
   
    printf("\n");
   
    printf(WSPACE_FMT "%s", "descriptors", "\n");
   
    // example descriptor
    print_commpage_descriptor("  mach_absolute_time()",
                              _COMM_PAGE_ABSOLUTE_TIME);
   
    exit(0);
}
