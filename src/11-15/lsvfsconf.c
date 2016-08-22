// lsvfsconf.c
   
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/errno.h>
   
void
print_flags(int f)
{
    if (f & MNT_LOCAL)    // file system is stored locally
        printf("local ");
    if (f & MNT_DOVOLFS)  // supports volfs
        printf("volfs ");
    printf("\n");
}
   
int
main(void)
{
    int    i, ret, val;
    size_t len;
    int    mib[4];
   
    struct vfsconf vfsconf;
   
    mib[0] = CTL_VFS;
   
    mib[1] = VFS_NUMMNTOPS; // retrieve number of mount/unmount operations
    len = sizeof(int);
    if ((ret = sysctl(mib, 2, &val, &len, NULL, 0)) < 0)
        goto out;
    printf("%d mount/unmount operations across all VFSs\n\n", val);
   
    mib[1] = VFS_GENERIC;
    mib[2] = VFS_MAXTYPENUM; // retrieve highest defined file system type
    len = sizeof(int);
    if ((ret = sysctl(mib, 3, &val, &len, NULL, 0)) < 0)
        goto out;
   
    mib[2] = VFS_CONF; // retrieve vfsconf for each type
    len = sizeof(vfsconf);
    printf("name        typenum refcount mountroot next     flags\n");
    printf("----        ------- -------- --------- ----     -----\n");
    for (i = 0; i < val; i++) {
        mib[3] = i;
        if ((ret = sysctl(mib, 4, &vfsconf, &len, NULL, 0)) != 0) {
            if (errno != ENOTSUP) // if error is ENOTSUP, let us ignore it
                goto out;
        } else {
            printf("%-11s %-7d %-8d %#09lx %#08lx ",
                   vfsconf.vfc_name, vfsconf.vfc_typenum, vfsconf.vfc_refcount,
                   (unsigned long)vfsconf.vfc_mountroot,
                   (unsigned long)vfsconf.vfc_next);
            print_flags(vfsconf.vfc_flags);
        }
    }
   
out:
    if (ret)
        perror("sysctl");
   
    exit(ret);
}
