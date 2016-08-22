; cntlzd.s
        .text
        .align 2
#ifndef __ppc64__
        .machine ppc970
#endif
        .globl _cntlzd
_cntlzd:
        cntlzd  r3,r3
        blr
