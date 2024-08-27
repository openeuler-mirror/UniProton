.global _start
.extern sys_stackEnd
.section .text
_start:
    .long 0x0100006f
    .long 0x15759620
    .long __data_end__
    .long __text_start__
r_start:
    la sp, sys_stackEnd
    call main
_spin:
    wfi 
    j _spin

