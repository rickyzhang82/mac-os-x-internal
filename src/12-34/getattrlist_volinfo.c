// getattrlist_volinfo.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/attr.h>
   
#define PROGNAME "getattrlist_volinfo"
   
// getattrlist() returns volume capabilities in this attribute buffer format
typedef struct {
    unsigned long           size;
    vol_capabilities_attr_t attributes;
} volinfo_buf_t;
   
// for pretty-printing convenience
typedef struct {
    u_int32_t   bits;
    const char *name; 
} bits_name_t;
   
#define BITS_NAME(bits) { bits, #bits }
   
// map feature availability bits to names
bits_name_t vol_capabilities_format[] = {
    BITS_NAME(VOL_CAP_FMT_2TB_FILESIZE),
    BITS_NAME(VOL_CAP_FMT_CASE_PRESERVING),
    BITS_NAME(VOL_CAP_FMT_CASE_SENSITIVE),
    BITS_NAME(VOL_CAP_FMT_FAST_STATFS),
    BITS_NAME(VOL_CAP_FMT_HARDLINKS),
    BITS_NAME(VOL_CAP_FMT_JOURNAL),
    BITS_NAME(VOL_CAP_FMT_JOURNAL_ACTIVE),
    BITS_NAME(VOL_CAP_FMT_NO_ROOT_TIMES),
    BITS_NAME(VOL_CAP_FMT_PERSISTENTOBJECTIDS),
    BITS_NAME(VOL_CAP_FMT_SYMBOLICLINKS),
    BITS_NAME(VOL_CAP_FMT_SPARSE_FILES),
    BITS_NAME(VOL_CAP_FMT_ZERO_RUNS),
};
#define VOL_CAP_FMT_SZ (sizeof(vol_capabilities_format)/sizeof(bits_name_t))
   
// map interface availability bits to names
bits_name_t vol_capabilities_interfaces[] = {
    BITS_NAME(VOL_CAP_INT_ADVLOCK),
    BITS_NAME(VOL_CAP_INT_ALLOCATE),
    BITS_NAME(VOL_CAP_INT_ATTRLIST),
    BITS_NAME(VOL_CAP_INT_COPYFILE),
    BITS_NAME(VOL_CAP_INT_EXCHANGEDATA),
    BITS_NAME(VOL_CAP_INT_EXTENDED_SECURITY),
    BITS_NAME(VOL_CAP_INT_FLOCK),
    BITS_NAME(VOL_CAP_INT_NFSEXPORT),
    BITS_NAME(VOL_CAP_INT_READDIRATTR),
    BITS_NAME(VOL_CAP_INT_SEARCHFS),
    BITS_NAME(VOL_CAP_INT_USERACCESS),
    BITS_NAME(VOL_CAP_INT_VOL_RENAME),
};
#define VOL_CAP_INT_SZ (sizeof(vol_capabilities_interfaces)/sizeof(bits_name_t))
   
void
print_volume_capabilities(volinfo_buf_t *volinfo_buf,
                          bits_name_t   *bits_names,
                          ssize_t        size,
                          u_int32_t      index)
{
    u_int32_t capabilities = volinfo_buf->attributes.capabilities[index];
    u_int32_t valid        = volinfo_buf->attributes.valid[index];
    int i;
   
    for (i = 0; i < size; i++)
        if ((bits_names[i].bits & valid) && (bits_names[i].bits & capabilities))
            printf("%s\n", bits_names[i].name);
    printf("\n");
}
   
int
main(int argc, char **argv)
{
    volinfo_buf_t   volinfo_buf;
    struct attrlist attrlist;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <volume path>\n", PROGNAME);
        exit(1);
    }
   
    // populate the ingoing attribute list structure
    attrlist.bitmapcount = ATTR_BIT_MAP_COUNT;    // always set to this constant
    attrlist.reserved    = 0;                     // reserved field zeroed
    attrlist.commonattr  = 0;                     // we don't want ATTR_CMN_*
    attrlist.volattr     = ATTR_VOL_CAPABILITIES; // we want these attributes
    attrlist.dirattr     = 0;                     // we don't want ATTR_DIR_*
    attrlist.fileattr    = 0;                     // we don't want ATTR_FILE_*
    attrlist.forkattr    = 0;                     // we don't want ATTR_FORK_*
   
    if (getattrlist(argv[1], &attrlist, &volinfo_buf, sizeof(volinfo_buf), 0)) {
        perror("getattrlist");
        exit(1);
    }
   
    print_volume_capabilities(&volinfo_buf,
                              (bits_name_t *)&vol_capabilities_format,
                              VOL_CAP_FMT_SZ, VOL_CAPABILITIES_FORMAT);
   
    print_volume_capabilities(&volinfo_buf,
                              (bits_name_t *)&vol_capabilities_interfaces,
                              VOL_CAP_INT_SZ, VOL_CAPABILITIES_INTERFACES);
   
    exit(0);
}
