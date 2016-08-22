; my_pthread_self_970.S
        .text
        .globl _my_pthread_self_970
_my_pthread_self_970:
        mfspr r3,259 ; 259 is user SPRG3
        blr
