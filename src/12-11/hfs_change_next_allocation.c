// hfs_change_next_allocation.c
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
   
// ensure that the following match the definitions in bsd/hfs/hfs_fsctl.h
// for the current kernel version, or include that header file directly
#define HFSIOC_CHANGE_NEXT_ALLOCATION  _IOWR('h', 3, u_int32_t)
#define HFS_CHANGE_NEXT_ALLOCATION  IOCBASECMD(HFSIOC_CHANGE_NEXT_ALLOCATION)
   
#define PROGNAME "hfs_change_next_allocation"
   
int
main(int argc, char **argv)
{
    int ret = -1;
    u_int32_t block_number, new_block_number;
   
    if (argc != 3) {
        fprintf(stderr, "usage: %s <volume path> <hexadecimal block number>\n",
                PROGNAME);
        exit(1);
    }
   
    block_number = strtoul(argv[2], NULL, 16);
    new_block_number = block_number;
    
    ret = fsctl(argv[1], HFS_CHANGE_NEXT_ALLOCATION, (void *)block_number, 0);
    if (ret)
        perror("fsctl");
    else
        printf("start block for next allocation search changed to %#x\n",
               new_block_number);
   
    exit(ret);
}
