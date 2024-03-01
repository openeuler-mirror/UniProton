.align 4
.global  OsTskContextLoad
.global  OsTaskTrap
.global  trap
.extern g_runningTask
.extern trap_entry
.extern OsMainSchedule
.extern intterupt_in
.extern OsHwiDispatchTail_rv64
.extern __os_sys_sp_end
.section .text
OsTskContextLoad:
    add t0, a0 ,zero
    ld  t0, 0(t0)
.ifdef OS_ARCH_SURPORT_F
    ld  t1, 0(t0)
    fscsr zero, t1
    fld fs11, 8(t0)
    fld fs10, 16(t0)
    fld fs9,  24(t0)
    fld fs8,  32(t0)
    fld fs7,  40(t0)
    fld fs6,  48(t0)
    fld fs5,  56(t0)
    fld fs4,  64(t0)
    fld fs3,  72(t0)
    fld fs2,  80(t0)
    fld fs1,  88(t0)
    fld fs0,  96(t0)
    ld  t1 ,  104(t0)
    csrw mepc, t1
    ld  t1 ,  112(t0)
    csrw mstatus, t1
    ld  t6 ,  120(t0)
    ld  t5 ,  128(t0)
    ld  t4 ,  136(t0)
    ld  t3 ,  144(t0)
    ld  s11 , 152(t0)
    ld  s10 , 160(t0)
    ld  s9  , 168(t0)
    ld  s8  , 176(t0)
    ld  s7  , 184(t0)
    ld  s6  , 192(t0)
    ld  s5  , 200(t0)
    ld  s4  , 208(t0)
    ld  s3  , 216(t0)
    ld  s2  , 224(t0)
    ld  a7  , 232(t0)
    ld  a6  , 240(t0)
    ld  a5  , 248(t0)
    ld  a4  , 256(t0)
    ld  a3  , 264(t0)
    ld  a2  , 272(t0)
    ld  a1  , 280(t0)
    ld  a0  , 288(t0)
    ld  s1  , 296(t0)
    ld  s0  , 304(t0)
    ld  t2  , 312(t0)
    ld  t1  , 320(t0)
    ld  tp  , 336(t0)
    ld  gp  , 344(t0)
    ld  ra  , 352(t0)
    add sp  , t0 , zero
    ld  t0  , 328(t0)
    addi sp, sp, 360
.else 
    ld  t1,  0(t0)
    csrw mepc, t1
    ld  t1,  8(t0)
    csrw mstatus, t1
    ld  t6, 16(t0)
    ld  t5, 24(t0)
    ld  t4, 32(t0)
    ld  t3, 40(t0)
    ld  s11, 48(t0)
    ld  s10, 56(t0)
    ld  s9, 64(t0)
    ld  s8, 72(t0)
    ld  s7, 80(t0)
    ld  s6, 88(t0)
    ld  s5, 96(t0)
    ld  s4, 104(t0)
    ld  s3, 112(t0)
    ld  s2, 120(t0)
    ld  a7, 128(t0)
    ld  a6, 136(t0)
    ld  a5, 144(t0)
    ld  a4, 152(t0)
    ld  a3, 160(t0)
    ld  a2, 168(t0)
    ld  a1, 176(t0)
    ld  a0, 184(t0)
    ld  s1, 192(t0)
    ld  s0, 200(t0)
    ld  t2, 208(t0)
    ld  t1, 216(t0)
    ld  tp, 232(t0)
    ld  gp, 240(t0)
    ld  ra, 248(t0)
    add sp, t0 , zero
    ld  t0, 224(t0)
    addi sp, sp, 256
.endif
    mret


.align 4
.section .text
trap:
.ifdef OS_ARCH_SURPORT_F
    addi sp, sp , -360
    sd   ra, 352(sp)
    sd   gp, 344(sp)
    sd   tp, 336(sp)
    sd   t0, 328(sp)
    sd   t1, 320(sp)
    sd   t2, 312(sp)
    sd   s0, 304(sp)
    sd   s1, 296(sp)
    sd   a0, 288(sp)
    sd   a1, 280(sp)
    sd   a2, 272(sp)
    sd   a3, 264(sp)
    sd   a4, 256(sp)
    sd   a5, 248(sp)
    sd   a6, 240(sp)
    sd   a7, 232(sp)
    sd   s2, 224(sp)
    sd   s3, 216(sp)
    sd   s4, 208(sp)
    sd   s5, 200(sp)
    sd   s6, 192(sp)
    sd   s7, 184(sp)
    sd   s8, 176(sp)
    sd   s9, 168(sp)
    sd  s10, 160(sp)
    sd  s11, 152(sp)
    sd   t3, 144(sp)
    sd   t4, 136(sp)
    sd   t5, 128(sp)
    sd   t6, 120(sp)
    csrr t0, mstatus
    sd   t0, 112(sp)
    csrr t0, mepc
    sd   t0, 104(sp)
    fsd fs0, 96(sp)
    fsd fs1, 88(sp)
    fsd fs2, 80(sp)
    fsd fs3, 72(sp)
    fsd fs4, 64(sp)
    fsd fs5, 56(sp)
    fsd fs6, 48(sp)
    fsd fs7, 40(sp)
    fsd fs8, 32(sp)
    fsd fs9, 24(sp)
    fsd fs10, 16(sp)
    fsd fs11, 8(sp)
    frcsr t0
    sd  t0,   0(sp)
.else
    addi sp, sp , -256
    sd   ra, 248(sp)
    sd   gp, 240(sp)
    sd   tp, 232(sp)
    sd   t0, 224(sp)
    sd   t1, 216(sp)
    sd   t2, 208(sp)
    sd   s0, 200(sp)
    sd   s1, 192(sp)
    sd   a0, 184(sp)
    sd   a1, 176(sp)
    sd   a2, 168(sp)
    sd   a3, 160(sp)
    sd   a4, 152(sp)
    sd   a5, 144(sp)
    sd   a6, 136(sp)
    sd   a7, 128(sp)
    sd   s2, 120(sp)
    sd   s3, 112(sp)
    sd   s4, 104(sp)
    sd   s5, 96(sp)
    sd   s6, 88(sp)
    sd   s7, 80(sp)
    sd   s8, 72(sp)
    sd   s9, 64(sp)
    sd  s10, 56(sp)
    sd  s11, 48(sp)
    sd   t3, 40(sp)
    sd   t4, 32(sp)
    sd   t5, 24(sp)
    sd   t6, 16(sp)
    csrr t0, mstatus
    sd   t0, 8(sp)
    csrr t0, mepc
    sd   t0, 0(sp)
.endif
#  检查一下是否中断嵌套 如果是我们借助 sys stack 来暂存中断的状态 而不是利用任务的栈
    .extern g_intCount
    la  t0, g_intCount
    lw  t0, 0(t0)
    bne t0, zero, regSaveEnd

#  将新的sp 存到 running task 中，方便下次捡起执行流
# ##################################################  
    la t0, g_runningTask
    ld t0, 0(t0)
    sd sp, 0(t0)
    la sp, __os_sys_sp_end # switch to sys stack
# ##################################################   

regSaveEnd:
    # init sys flag
    csrr a0, mcause
    call intterupt_in

    # for riscv trap
    csrr a0, mcause
    call trap_entry

    # 中斷尾部 鈎子處理以及tick相關
    call OsHwiDispatchTail_rv64

    # clear the sys flag
    csrr a0, mcause
    call intterupt_out

    #  检查一下是否中断嵌套 是中断嵌套我们应该直接借助 sys stack 回去 像跳转函数一样
    la  t0, g_intCount
    lw  t0, 0(t0)
    bne t0, zero, intNestRet
    
    call OsMainSchedule
    j _err_spin
intNestRet:
.ifdef OS_ARCH_SURPORT_F 
    ld  t1, 0(sp)
    fscsr zero, t1
    fld fs11, 8(sp)
    fld fs10, 16(sp)
    fld fs9,  24(sp)
    fld fs8,  32(sp)
    fld fs7,  40(sp)
    fld fs6,  48(sp)
    fld fs5,  56(sp)
    fld fs4,  64(sp)
    fld fs3,  72(sp)
    fld fs2,  80(sp)
    fld fs1,  88(sp)
    fld fs0,  96(sp)
    ld  t1 ,  104(sp)
    csrw mepc, t1
    ld  t1 ,  112(sp)
    csrw mstatus, t1
    ld  t6 ,  120(sp)
    ld  t5 ,  128(sp)
    ld  t4 ,  136(sp)
    ld  t3 ,  144(sp)
    ld  s11 , 152(sp)
    ld  s10 , 160(sp)
    ld  s9  , 168(sp)
    ld  s8  , 176(sp)
    ld  s7  , 184(sp)
    ld  s6  , 192(sp)
    ld  s5  , 200(sp)
    ld  s4  , 208(sp)
    ld  s3  , 216(sp)
    ld  s2  , 224(sp)
    ld  a7  , 232(sp)
    ld  a6  , 240(sp)
    ld  a5  , 248(sp)
    ld  a4  , 256(sp)
    ld  a3  , 264(sp)
    ld  a2  , 272(sp)
    ld  a1  , 280(sp)
    ld  a0  , 288(sp)
    ld  s1  , 296(sp)
    ld  s0  , 304(sp)
    ld  t2  , 312(sp)
    ld  t1  , 320(sp)
    ld  t0  , 328(sp)
    ld  tp  , 336(sp)
    ld  gp  , 344(sp)
    ld  ra  , 352(sp)
    addi sp, sp, 360
.else 
    ld  t1,  0(sp)
    csrw mepc, t1
    ld  t1,  8(sp)
    csrw mstatus, t1
    ld  t6, 16(sp)
    ld  t5, 24(sp)
    ld  t4, 32(sp)
    ld  t3, 40(sp)
    ld  s11, 48(sp)
    ld  s10, 56(sp)
    ld  s9, 64(sp)
    ld  s8, 72(sp)
    ld  s7, 80(sp)
    ld  s6, 88(sp)
    ld  s5, 96(sp)
    ld  s4, 104(sp)
    ld  s3, 112(sp)
    ld  s2, 120(sp)
    ld  a7, 128(sp)
    ld  a6, 136(sp)
    ld  a5, 144(sp)
    ld  a4, 152(sp)
    ld  a3, 160(sp)
    ld  a2, 168(sp)
    ld  a1, 176(sp)
    ld  a0, 184(sp)
    ld  s1, 192(sp)
    ld  s0, 200(sp)
    ld  t2, 208(sp)
    ld  t1, 216(sp)
    ld  t0, 224(sp)
    ld  tp, 232(sp)
    ld  gp, 240(sp)
    ld  ra, 248(sp)
    addi sp, sp, 256
.endif
    mret
_err_spin:
    wfi
    j _err_spin

.align 4
.section .text
OsTaskTrap:
    # 这里要模拟一下trap 的时候对mstatus 的操作
    # 若没有模拟， 进行mret 返回的是硬件状态是不确定的，可能是S也可能是M可能是U
    # 1. 根据 MIE 的状态写进 MPIE里面
    # 2. 把 MIE 清0
    # 3. 把 MPP 的值设为 0b11 也就是进行mret后的硬件状态
    # 4. 来到Trap 的M态 [这个不用做 因为状态是扁平化的，在trap的时候没有跃迁 现在就是M态]
.ifdef OS_ARCH_SURPORT_F
    addi sp, sp, -360
    sd   t0, 328(sp)
    sd   t1, 320(sp)
.else 
    addi sp, sp, -256
    sd   t0, 224(sp)
    sd   t1, 216(sp)
.endif
    # 存好t0后暂时借用 t0 来模拟trap的行为
    # t1 用来存放立即数
    csrr t0, mstatus
    andi t0, t0, 0x8
    beq  t0, zero, bef_no_int
bef_can_int:
    li   t1, 0x80
    csrrs zero, mstatus, t1
    j bef_int_end
bef_no_int:
    li   t1, 0x80
    csrrc zero, mstatus, t1
bef_int_end:
    # 把 MIE 放到 MPIE 后，马上把MIE清0
    li   t1, 0x8
    csrrc zero, mstatus, t1
    # 把 MPP 置为 0b11
    li   t1, 0x1800
    csrrs zero, mstatus, t1
    # ###############

.ifdef OS_ARCH_SURPORT_F
    sd   ra, 352(sp)
    sd   gp, 344(sp)
    sd   tp, 336(sp)
    # sd   t0, 328(sp)
    # sd   t1, 320(sp)
    sd   t2, 312(sp)
    sd   s0, 304(sp)
    sd   s1, 296(sp)
    sd   a0, 288(sp)
    sd   a1, 280(sp)
    sd   a2, 272(sp)
    sd   a3, 264(sp)
    sd   a4, 256(sp)
    sd   a5, 248(sp)
    sd   a6, 240(sp)
    sd   a7, 232(sp)
    sd   s2, 224(sp)
    sd   s3, 216(sp)
    sd   s4, 208(sp)
    sd   s5, 200(sp)
    sd   s6, 192(sp)
    sd   s7, 184(sp)
    sd   s8, 176(sp)
    sd   s9, 168(sp)
    sd  s10, 160(sp)
    sd  s11, 152(sp)
    sd   t3, 144(sp)
    sd   t4, 136(sp)
    sd   t5, 128(sp)
    sd   t6, 120(sp)
    csrr t0, mstatus
    sd   t0, 112(sp)
    la   t0, _tsk_trap_ret
    sd   t0, 104(sp)    # 此处就应该使用ra 而不是mepc来作为上下文中的mepc
    fsd fs0, 96(sp)
    fsd fs1, 88(sp)
    fsd fs2, 80(sp)
    fsd fs3, 72(sp)
    fsd fs4, 64(sp)
    fsd fs5, 56(sp)
    fsd fs6, 48(sp)
    fsd fs7, 40(sp)
    fsd fs8, 32(sp)
    fsd fs9, 24(sp)
    fsd fs10, 16(sp)
    fsd fs11, 8(sp)
    frcsr t0
    sd  t0,   0(sp)
.else 
    sd   ra, 248(sp)
    sd   gp, 240(sp)
    sd   tp, 232(sp)
  # sd   t0, 224(sp)
  # sd   t1, 216(sp)
    sd   t2, 208(sp)
    sd   s0, 200(sp)
    sd   s1, 192(sp)
    sd   a0, 184(sp)
    sd   a1, 176(sp)
    sd   a2, 168(sp)
    sd   a3, 160(sp)
    sd   a4, 152(sp)
    sd   a5, 144(sp)
    sd   a6, 136(sp)
    sd   a7, 128(sp)
    sd   s2, 120(sp)
    sd   s3, 112(sp)
    sd   s4, 104(sp)
    sd   s5, 96(sp)
    sd   s6, 88(sp)
    sd   s7, 80(sp)
    sd   s8, 72(sp)
    sd   s9, 64(sp)
    sd  s10, 56(sp)
    sd  s11, 48(sp)
    sd   t3, 40(sp)
    sd   t4, 32(sp)
    sd   t5, 24(sp)
    sd   t6, 16(sp)
    csrr t0, mstatus
    sd   t0, 8(sp)
    la   t0, _tsk_trap_ret
    sd   t0, 0(sp)    # 此处就应该使用ra 而不是mepc来作为上下文中的mepc
.endif
    la t0, g_runningTask
    ld t0, 0(t0)
    sd sp, 0(t0)
    la sp, __os_sys_sp_end # switch to sys stack
    call OsMainSchedule
_err_spin_2:
    ecall
    j _err_spin_2
_tsk_trap_ret:
    ret
