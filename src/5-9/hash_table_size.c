// hash_table_size.c
   
#define PROGNAME "hash_table_size"
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mach/vm_region.h>
   
typedef unsigned int uint_t;
   
#define PTEG_SIZE_G4 64
#define PTEG_SIZE_G5 128
   
extern unsigned int cntlzw(unsigned int num);
   
vm_size_t
calculate_hash_table_size(uint64_t msize, int pfPTEG, int hash_table_shift)
{
    unsigned int nbits;
    uint64_t     tmemsize;
    vm_size_t    hash_table_size;
   
    // Get first bit in upper half
    nbits = cntlzw(((msize << 1) - 1) >> 32);
   
    // If upper half is empty, find bit in lower half
    if (nbits == 32)
        nbits = nbits + cntlzw((uint_t)((msize << 1) - 1));
   
    // Get memory size rounded up to a power of 2
    tmemsize = 0x8000000000000000ULL >> nbits;
   
    // Ensure 32-bit arithmetic doesn't overflow
    if (tmemsize > 0x0000002000000000ULL)
        tmemsize = 0x0000002000000000ULL;
   
    // IBM-recommended hash table size (1 PTEG per 2 physical pages)
    hash_table_size = (uint_t)(tmemsize >> (12 + 1)) * pfPTEG;
   
    // Mac OS X uses half of the IBM-recommended size
    hash_table_size >>= 1;
   
    // Apply ht_shift, if necessary
    if (hash_table_shift >= 0) // make size bigger
        hash_table_size <<= hash_table_shift;
    else // Make size smaller
        hash_table_size >>= (-hash_table_shift);
   
    // Ensure minimum size
    if (hash_table_size < (256 * 1024))
        hash_table_size = (256 * 1024);
   
    return hash_table_size;
}
   
int
main(int argc, char **argv)
{
    vm_size_t htsize;
    uint64_t msize; 
   
    if (argc != 2) {
        fprintf(stderr, "%s <memory in MB>\n", PROGNAME);
        exit(1);
    }
   
    msize = ((uint64_t)(atoi(argv[1])) << 20);
    htsize = calculate_hash_table_size(msize, PTEG_SIZE_G5, 0);
   
    printf("%d bytes (%dMB)\n", htsize, htsize >> 20);
   
    exit(0);
}
