.global _start
.extern sys_stackEnd
.section .text
_start:
    csrr t0, mhartid
    bne t0, zero, core_1
    la sp, sys_stackEnd
    # init the fs in mstatus to enable f/d instructions
    li t0, 0x2000
    csrrs zero, mstatus, t0
    call main
_spin:
    wfi 
    j _spin
core_1:
    li t0, 0x92C21000
    jr t0
    j _spin
