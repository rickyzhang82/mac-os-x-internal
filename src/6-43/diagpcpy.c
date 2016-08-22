// diagpcpy.c
   
#include "diagCommon.h"
   
#define PROGNAME "diagpcpy"
   
void usage(void);
   
int
main(int argc, char **argv)
{
    int        ret;
    u_int32_t  phys;
    u_int32_t  nbytes;
    char      *buffer;
   
    if (argc != 3)
        usage();
   
    phys = strtoul(argv[1], NULL, 16);
    nbytes = strtoul(argv[2], NULL, 10);
    if ((nbytes < 0) || (phys < 0))
        usage();
   
    nbytes = (nbytes > MAXBYTES) ? MAXBYTES : nbytes;
    buffer = (char *)malloc(nbytes);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }
   
    // copy physical to virtual
    ret = diagCall_(dgpcpy, 0, phys, 0, buffer, nbytes, cppvPsrc|cppvNoRefSrc);
   
    (void)write(1, buffer, nbytes);
               
    free(buffer);
   
    exit(0);
}
   
void
usage(void)
{
    printf("usage: %s <physical addr> <bytes>\n", PROGNAME);
    printf("\tphysical address must be specified in hexadecimal\n");
    printf("\tnumber of bytes to copy must be specified in decimal\n");
   
    exit(1);
}
