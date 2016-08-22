; cntlzw.s
; count leading zeros in a 32-bit word
;
        .text
        .align 4
        .globl _cntlzw
_cntlzw:
        cntlzw r3,r3
        blr
