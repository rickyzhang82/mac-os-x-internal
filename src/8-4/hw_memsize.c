// hw_memsize.c
   
#include <stdio.h>
#include <sys/sysctl.h>
   
int
main(void)
{
    int                ret;
    unsigned long long memsize;
    size_t             len = sizeof(memsize);
   
    if (!(ret = sysctlbyname("hw.memsize", &memsize, &len, NULL, 0)))
        printf("%lld MB\n", (memsize >> 20ULL));
    else
        perror("sysctlbyname");
   
    return ret; 
}
