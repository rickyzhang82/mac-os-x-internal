// shm_info.c
   
#include "shm_common.h"
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
   
#define PROGNAME "shm_info"
   
void
print_stat_info(char *name, struct stat *sb)
{
    struct passwd *passwd;
    struct group  *group;
    char           filemode[11 + 1];
   
    passwd = getpwuid(sb->st_uid);
    group = getgrgid(sb->st_gid);
    strmode(sb->st_mode, filemode);
   
    printf("%s  ", filemode);
   
    if (passwd)
        printf("%s  ", passwd->pw_name);
    else
        printf("%d  ", sb->st_uid);
    if (group)
        printf("%s  ", group->gr_name);
    else
        printf("%d  ", sb->st_gid);
   
    printf("%u %s\n", (unsigned int)sb->st_size, name);
}
   
int
main(int argc, char **argv)
{
    char        *p;
    int          shm_fd;
    struct stat  sb;
   
    CHECK_ARGS(2, "<path>");
   
    if ((shm_fd = shm_open(argv[1], 0)) < 0) {
        perror("shm_open");
        exit(1);
    }
   
    if (fstat(shm_fd, &sb)) {
        perror("fstat");
        exit(1);
    }
   
    print_stat_info(argv[1], &sb);
   
    p = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, shm_fd, 0); 
    printf("Contents: %s\n", p);
    munmap(p, sb.st_size);
   
    close(shm_fd);
   
    exit(0);
}
