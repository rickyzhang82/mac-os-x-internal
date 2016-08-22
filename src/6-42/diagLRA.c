// diagLRA.c
   
#include "diagCommon.h"
   
#define PROGNAME "diagLRA"
   
int
main(int argc, char **argv)
{
    u_int32_t phys, virt;
    u_int64_t physaddr;
   
    if (argc != 2) {
        printf("usage: %s <virtual address in hex>\n", PROGNAME);
        exit(1);
    }
   
    // Must be in hexadecimal
    virt = strtoul(argv[1], NULL, 16);
   
    phys = diagCall_(dgLRA, virt);
   
    if (!phys) {
        printf("virtual address %08X :: physical page none\n", virt);
        exit(1);
    }
   
    physaddr = (u_int64_t)phys * 0x1000ULL + (u_int64_t)(virt & 0xFFF);
    printf("virtual address %#08X :: physical page %#X (address %#llX)\n",
            virt, phys, physaddr);
               
    exit(0);
}
