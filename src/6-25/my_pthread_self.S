; my_pthread_self.S
        .text
        .globl _my_pthread_self
_my_pthread_self:
        li r0,0x7FF2
        sc
        blr
