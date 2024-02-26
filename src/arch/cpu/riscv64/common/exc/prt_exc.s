.macro LOADEXECINFO
    .extern g_excCalleeInfo
    .extern g_causeRegInfo
    la t0 , g_excCalleeInfo
    sd sp , 0(t0)
    sd s0 , 8(t0)
    sd fp , 16(t0)
    sd s2 , 24(t0)
    sd s3 , 32(t0)
    sd s4 , 40(t0)
    sd s5 , 48(t0)
    sd s6 , 56(t0)
    sd s7 , 64(t0)
    sd s8 , 72(t0)
    sd s9 , 80(t0)
    sd s10, 88(t0)
    sd s11, 96(t0)  #上面是在保存callee寄存器到全局变量g_excCalleeInfo
    la t0 , g_causeRegInfo 
    csrr t1 , mcause
    sd t1 , 0(t0)
    csrr t1 , mepc
    sd t1 , 8(t0)
    csrr t1 , mstatus
    sd t1 , 16(t0)
    csrr t1 , mtval
    sd t1 , 24(t0)  #上面是在保存异常状态相关的寄存器到全局变量g_causeRegInfo
.endm

.section .text
.align 4
.global OsExcDispatch #异常分发函数 - 异常处理函数 ,硬件异常的处理函数入口Entry
OsExcDispatch:
    LOADEXECINFO
    .extern g_excInfoInternal
    .extern OsExcSaveInfo
    .extern OsExcHandleEntryRISCV
    addi sp, sp, -8
    sd   ra, 0(sp)
    la a0 , g_excInfoInternal
    la a1 , g_excCalleeInfo
    la a2 , g_causeRegInfo
    call OsExcSaveInfo
    call OsExcHandleEntryRISCV
    ld   ra, 0(sp)
    addi sp, sp, 8
    ret
    