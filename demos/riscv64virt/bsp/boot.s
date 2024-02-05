.global _start
.extern sys_stackEnd
.section .text
_start:
    la sp, sys_stackEnd
    call main
_spin:
    wfi 
    j _spin
    j _start

