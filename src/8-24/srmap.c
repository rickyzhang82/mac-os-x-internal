// srmap.c
// maps a 32-bit, non-fat, dynamic shared library into the system shared region
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <mach-o/loader.h>
#include <mach/shared_memory_server.h>
   
#define PROGNAME "srmap"
   
struct _shared_region_mapping_np {
    mach_vm_address_t address;
    mach_vm_size_t    size;
    mach_vm_offset_t  file_offset;
    vm_prot_t         max_prot;   // VM_PROT_{READ/WRITE/EXECUTE/COW/ZF}
    vm_prot_t         init_prot;  // VM_PROT_{READ/WRITE/EXECUTE/COW/ZF}
};
typedef struct _shared_region_mapping_np sr_mapping_t;
#define MAX_SEGMENTS 64
   
// shared_region_map_file_np() is not exported through libSystem in
// Mac OS X 10.4, so we use the indirect system call to call it
int
_shared_region_map_file_np(int fd,
                           unsigned int nregions,
                           sr_mapping_t regions[],
                           uint64_t *slide)
{
    return syscall(SYS_shared_region_map_file_np, fd, nregions, regions, slide);
}
   
   
int
main(int argc, char **argv)
{
    int                     fd, ret = 1;
    struct mach_header     *mh;                 // pointer to the Mach-O header
    char                   *load_commands;      // buffer for load commands
    uint32_t                ncmds;              // number of load commands
    struct load_command    *lc;                 // a particular load command
    struct segment_command *sc;                 // a particular segment command
    uint64_t                vmaddr_slide;       // slide value from the kernel
    void                   *load_address = 0;   // for mmaping the Mach-O file
    unsigned                int entryIndex = 0; // index into the mapping table
    sr_mapping_t            mappingTable[MAX_SEGMENTS], *entry;
    uintptr_t               base_address = (uintptr_t)ULONG_MAX;
    uint64_t                file_length;
    struct stat             sb;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <library path>\n", PROGNAME);
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
   
    // determine the file's size
    if (fstat(fd, &sb))
        goto OUT;
    file_length = sb.st_size;
   
    // get a locally mapped copy of the file
    load_address = mmap(NULL, file_length, PROT_READ, MAP_FILE, fd, 0);
    if (load_address == ((void *)(-1)))
        goto OUT;
   
    // check out the Mach-O header
    mh = (struct mach_header *)load_address;
   
    if ((mh->magic != MH_MAGIC) && (mh->filetype != MH_DYLIB)) {
        fprintf(stderr, "%s is not a Mach-O dynamic shared library\n", argv[1]);
        goto OUT;
    }
   
    if (!(mh->flags & MH_SPLIT_SEGS)) {
        fprintf(stderr, "%s does not use split segments\n", argv[1]);
        goto OUT;
    }
   
    load_commands = (char *)((char *)load_address + sizeof(struct mach_header));
    lc = (struct load_command *)load_commands;
   
    // process all LC_SEGMENT commands and construct a mapping table
    for (ncmds = mh->ncmds; ncmds > 0; ncmds--) {
        if (lc->cmd == LC_SEGMENT) {
            sc = (struct segment_command *)lc;
   
            // remember the starting address of the first segment (seg1addr)
            if (sc->vmaddr < base_address)
                base_address = sc->vmaddr;
   
            entry              = &mappingTable[entryIndex];
            entry->address     = sc->vmaddr;
            entry->size        = sc->filesize;
            entry->file_offset = sc->fileoff;
   
            entry->init_prot = VM_PROT_NONE;
            if (sc->initprot & VM_PROT_EXECUTE)
                entry->init_prot |= VM_PROT_EXECUTE;
            if (sc->initprot & VM_PROT_READ)
                entry->init_prot |= VM_PROT_READ;
            if (sc->initprot & VM_PROT_WRITE)
                entry->init_prot |= VM_PROT_WRITE | VM_PROT_COW;
   
            entry->max_prot = entry->init_prot;
   
            // check if the segment has a zero-fill area: if so, need a mapping
            if ((sc->initprot & VM_PROT_WRITE) && (sc->vmsize > sc->filesize)) {
                sr_mapping_t *zf_entry = &mappingTable[++entryIndex];
                zf_entry->address      = entry->address + sc->filesize;
                zf_entry->size         = sc->vmsize - sc->filesize;
                zf_entry->file_offset  = 0;
                zf_entry->init_prot    = entry->init_prot | \
                                             VM_PROT_COW | VM_PROT_ZF;
                zf_entry->max_prot     = zf_entry->init_prot;
            }
            entryIndex++;
        }
        // onto the next load command
        lc = (struct load_command *)((char *)lc + lc->cmdsize);
    }
   
    ret = _shared_region_map_file_np(fd,             // the file
                                     entryIndex,     // so many mappings
                                     mappingTable,   // the mappings
                                     &vmaddr_slide); // OK to slide, let us know
    if (!ret) { // success
        printf("mapping succeeded: base = %#08lx, slide = %#llx\n",
               base_address, vmaddr_slide);
    }
   
OUT:
    close(fd);
   
    exit(ret);
}
