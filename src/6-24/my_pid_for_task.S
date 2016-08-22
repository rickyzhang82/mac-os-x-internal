        .text
        .align 4
        .globl _pid_for_task
_pid_for_task:
        li r0,-46
        sc      
        blr     
