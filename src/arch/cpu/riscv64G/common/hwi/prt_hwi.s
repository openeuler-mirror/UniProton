.align 4
.global hwi_handler
.global hwi_timer_handler
.section .text
hwi_handler:
    .extern OsHwiGetIrqNumber
    addi sp, sp, -8
    sd   ra, 0(sp)
    call OsHwiGetIrqNumber
    add a0, a0, zero
    call OsHwiDispatchHandle
    ld   ra, 0(sp)
    addi sp, sp, 8
    ret

hwi_timer_handler:
    .extern HwTimerIsr
    .extern OsHwiDispatchTail
    addi sp, sp, -8
    sd   ra, 0(sp) 
    call HwTimerIsr
    ld   ra, 0(sp)
    addi sp, sp, 8
    ret 