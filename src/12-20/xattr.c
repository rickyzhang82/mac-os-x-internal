// xattr.c
   
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/xattr.h>
   
#define PROGNAME "xattr"
   
void
usage()
{
    fprintf(stderr, "\
Set or remove extended attributes. Usage:\n\n\
    %s -s <attribute-name>=<attribute-value> <filename> # set\n\
    %s -r <attribute-name> <filename>                   # remove\n\n\
    Notes: <attribute-name> must not contain a '=' character\n\
           <filename> must be the last argument\n", PROGNAME, PROGNAME);
    exit(1);
}
   
int
main(int argc, char **argv)
{
    size_t     size;
    u_int32_t  position = 0;
    int        ch, ret, options = XATTR_NOFOLLOW;
    char      *path = NULL, *name = NULL, *value = NULL;
   
    if (argc != 4)
        usage();
   
    path = argv[argc -  1];
    argc--;
   
    while ((ch = getopt(argc, argv, "r:s:")) != -1) {
        switch (ch) {
        case 'r':
            if (ret = removexattr(path, optarg, options))
                perror("removexattr");
            break;
   
        case 's':
            name = optarg;
            if ((value = strchr(optarg, '=')) == NULL)
                usage();
            *value = '\0';
            value++;
            size = strlen(value) + 1;
            if (ret = setxattr(path, name, value, size, position, options))
                perror("setxattr");
            break;
   
        default:
            usage();
        }
    }
   
    exit(ret);
}
