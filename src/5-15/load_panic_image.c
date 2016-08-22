// load_panic_image.c
   
#define PROGNAME "load_panic_image"
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
   
int
main(int argc, char **argv)
{
    int     ret, fd;
    char   *buf;
    size_t  oldlen = 0, newlen;
    struct  stat sb;
    int     mib[3] = { CTL_KERN, KERN_PANICINFO, KERN_PANICINFO_IMAGE };
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <kraw image file path>\n", PROGNAME);
        exit(1);
    }
   
    if (stat(argv[1], &sb) < 0) {
        perror("stat");
        exit(1);
    }
   
    newlen = sb.st_size;
    buf = (char *)malloc(newlen); // assume success
   
    fd = open(argv[1], O_RDONLY); // assume success
    ret = read(fd, buf, sb.st_size); // assume success
    close(fd);
   
    if (sysctl(mib, 3, NULL, (void *)&oldlen, buf, newlen))
        perror("sysctl");
   
    exit(ret);
}
