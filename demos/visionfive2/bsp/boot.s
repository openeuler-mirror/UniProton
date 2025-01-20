.global _start
.extern sys_stackEnd
.section .text
_start:
    li t0, 0x2000
    csrrs zero, mstatus, t0
    csrrs t0, mhartid, zero
    li t1, 1
    bne t1, t0, _spin
    la sp, sys_stackEnd
    j main
_spin:
    wfi 
    j _spin
