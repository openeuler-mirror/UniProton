.global _start
.extern sys_stackEnd
.section .text
_start:
    la sp, sys_stackEnd
    # init the fs in mstatus to enable f/d instructions
    li t0, 0x2000
    csrrs zero, mstatus, t0
    call main
_spin:
    wfi 
    j _spin
