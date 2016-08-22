// mach_task_self.c
   
#include <stdio.h>
#include <mach/mach.h>
#include <mach/mach_traps.h>
   
int
main(void)
{
    printf("%#x\n", mach_task_self());
#undef mach_task_self
    printf("%#x\n", mach_task_self());
    printf("%#x\n", task_self_trap());
    printf("%#x\n", mach_task_self_);
   
    return 0;
}
