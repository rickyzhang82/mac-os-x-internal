// getfwpath.c
   
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/disk.h>
   
#define PROGNAME "getfwpath"
   
int
main(int argc, char **argv)
{
    int fd;
    dk_firmware_path_t path = { { 0 } };
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <path>\n", PROGNAME);
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }
   
    if (ioctl(fd, DKIOCGETFIRMWAREPATH, &path) < 0) {
        perror("ioctl");
        close(fd);
        exit(1);
    }
   
    printf("%s\n", path.path);
   
    close(fd);
    exit(0);
}
