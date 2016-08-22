// panic_test.c
   
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
   
#define KERN_PANICINFO_TEST (KERN_PANICINFO_IMAGE + 2)
   
int
main(void)
{
    int ret;
    size_t oldnewlen = 0;
    int mib[3] = { CTL_KERN, KERN_PANICINFO, KERN_PANICINFO_TEST };
   
    ret = sysctl(mib, 3, NULL, (void *)&oldnewlen, NULL, oldnewlen);
   
    exit(ret);
}
