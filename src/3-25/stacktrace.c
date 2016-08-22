// stacktrace.c
   
#include <stdio.h>
#include <dlfcn.h>
   
void
printframeinfo(unsigned int level, void *fp, void *ra)
{
    int     ret;
    Dl_info info;
   
    // Find the image containing the given address
    ret = dladdr(ra, &info);
    printf("#%u %s%s in %s, fp = %p, pc = %p\n",
           level,
           (ret) ? info.dli_sname : "?",          // symbol name
           (ret) ? "()" : "",                     // show as a function
           (ret) ? info.dli_fname : "?", fp, ra); // shared object name
}
   
void
stacktrace()
{
    unsigned int level = 0;
    void    *saved_ra  = __builtin_return_address(0);
    void   **fp        = (void **)__builtin_frame_address(0);
    void    *saved_fp  = __builtin_frame_address(1);
   
    printframeinfo(level, saved_fp, saved_ra);
    level++;
    fp = saved_fp;
    while (fp) {
        saved_fp = *fp;
        fp = saved_fp;
        if (*fp == NULL)
            break;
        saved_ra = *(fp + 2);
        printframeinfo(level, saved_fp, saved_ra);
        level++;
    }
}
   
void f4() { stacktrace(); }
void f3() { f4(); }
void f2() { f3(); }
void f1() { f2(); }
   
int
main()
{
    f1();
    return 0;
}
