// diskinfo.c
   
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/disk.h>
   
#define PROGNAME "diskinfo"
   
void
cleanup(char *errmsg, int retval)
{
    perror(errmsg);
    exit(retval);
}
   
#define TRY_IOCTL(fd, request, argp) \
    if ((ret = ioctl(fd, request, argp)) < 0) { \
        close(fd); cleanup("ioctl", ret); \
    }
   
int
main(int argc, char **argv)
{
    int       fd, ret;
    u_int32_t blockSize;
    u_int64_t blockCount;
    u_int64_t maxBlockRead;
    u_int64_t maxBlockWrite;
    u_int64_t capacity1000, capacity1024;
   
    dk_firmware_path_t fwPath;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <raw disk>\n", PROGNAME);
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
        cleanup("open", 1);
   
    TRY_IOCTL(fd, DKIOCGETFIRMWAREPATH, &fwPath);
    TRY_IOCTL(fd, DKIOCGETBLOCKSIZE, &blockSize);
    TRY_IOCTL(fd, DKIOCGETBLOCKCOUNT, &blockCount);
    TRY_IOCTL(fd, DKIOCGETMAXBLOCKCOUNTREAD, &maxBlockRead);
    TRY_IOCTL(fd, DKIOCGETMAXBLOCKCOUNTWRITE, &maxBlockWrite);
   
    close(fd);
   
    capacity1024  = (blockCount * blockSize) / (1ULL << 30ULL);
    capacity1000  = (blockCount * blockSize) / (1000ULL * 1000ULL * 1000ULL);
    printf("%-20s = %s\n", "Device", argv[1]);
    printf("%-20s = %s\n", "Firmware Path", fwPath.path);
    printf("%-20s = %llu GB / %llu GiB\n", "Capacity",
           capacity1000, capacity1024);
    printf("%-20s = %u bytes\n", "Block Size", blockSize);
    printf("%-20s = %llu\n", "Block Count", blockCount);
    printf("%-20s = { read = %llu blocks, write = %llu blocks }\n",
           "Maximum Request Size", maxBlockRead, maxBlockWrite);
   
    exit(0);
}
