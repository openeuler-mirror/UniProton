/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: 线程调度的汇编部分。
 */
#include "prt_buildef.h"
#include "prt_asm_arm_external.h"

    .global g_sysStackHigh
    .global OsHwiDispatch, OsMainSchedule
    .global OsRunningReadyAdd

    .type   OsMainSchedule, function

// save x2~x30, xzr
.macro GENERAL_REGS_SAVE
    stp x3, x2, [sp,#-16]!
    stp x5, x4, [sp,#-16]!
    stp x7, x6, [sp,#-16]!
    stp x9, x8, [sp,#-16]!
    stp x11, x10, [sp,#-16]!
    stp x13, x12, [sp,#-16]!
    stp x15, x14, [sp,#-16]!
    stp x17, x16, [sp,#-16]!
    stp x19, x18, [sp,#-16]!
    stp x21, x20, [sp,#-16]!
    stp x23, x22, [sp,#-16]!
    stp x25, x24, [sp,#-16]!
    stp x27, x26, [sp,#-16]!
    stp x29, x28, [sp,#-16]!
    stp xzr, x30, [sp,#-16]!
.endm

    .section .os.text, "ax"

#if !defined(OS_OPTION_TASK_AFFINITY_STATIC)
/*
 * 描述: OsContextSave(TSK_CB_S *taskCB, bool isCrg); 忽略isCrg
 */
    .globl OsContextSave
    .type OsContextSave, @function
    .align 4
OsContextSave:
    stp x1, x0, [sp,#-16]!
    GENERAL_REGS_SAVE
    mrs x3, spsr_el1
    mov x2, x30 // 用返回地址x30作为现场恢复点
    sub sp, sp, #16 // 跳过esr_el1, far_el1, 异常时才有用
    stp x2, x3, [sp,#-16]!

    // 存入SP指针到g_pRunningTask->sp
    mov x1, sp
    str x1, [x0]  // x0 is the &g_pRunningTask->sp
#if defined(OS_OPTION_SMP)
    OsAsmGetCoreId x1  // 读取核号
    ldr x2, =g_sysStackHigh
    ldr x2, [x2, x1, lsl #3]
#else
    ldr x2, =g_sysStackHigh
#endif
    mov sp, x2
#if defined(OS_OPTION_SMP)
    BL OsRunningReadyAdd
#endif
    BL OsMainSchedule

saveloop:           // never return here
    B saveloop
#endif

/*
 * 描述: Task调度处理函数。 AMP调度走这个接口
 */
    .global OsTskTrapSmp
    .globl OsTaskTrap
    .type OsTskTrapSmp, @function
    .type OsTaskTrap, @function
    .align 4

#if !defined(OS_OPTION_SMP)
OsTaskTrap:
    LDR    x1, =g_runningTask /* OsTaskTrap是函数调用过来，x0 x1寄存器是caller save，此处能直接使用 */
    LDR    x0, [x1] /* x0 is the &g_pRunningTask->sp */
#endif
/*
 * 描述: Task调度处理函数。 SMP调度走这个接口，X0 is SP
 */
OsTskTrapSmp:
    stp    x1, x0, [sp,#-16]!
    GENERAL_REGS_SAVE

    /* TskTrap需要保存CPSR，由于不能直接访问，需要拼接获取当前CPSR入栈 */
    mrs    x3, DAIF /* CPSR：DAIF 4种事件的mask, bits[9:6] */
    mrs    x2, NZCV /* NZCV：Condition flags, bits[31:28] */
    orr    x3, x3, x2
    orr    x3, x3, #(0x1U << 2) /* 当前的 exception level,bits[3:2] 00:EL0,01:El1,10:El2,11:EL3 */
    orr    x3, x3, #(0x1U) /* 当前栈的选择,bits[0] 0:SP_EL0,1:SP_ELX */

    mov    x2, x30    // 用返回地址x30作为现场恢复点
    sub    sp, sp, #16  // 跳过esr_el1, far_el1, 异常时才有用
    stp    x2, x3, [sp,#-16]!

    // 存入SP指针到g_pRunningTask->sp
    mov    x1, sp
    str    x1, [x0]   // x0 is the &g_pRunningTask->sp

#if defined(OS_OPTION_SMP)
    // 获取cpuid号 算出系统偏移
    OsAsmGetCoreId x1   // 读取核号
    ldr    x0, =g_sysStackHigh
    ldr    x0, [x0, x1, lsl #3]
#else
    ldr    x0, =g_sysStackHigh
    ldr    x0, [x0]
#endif
    mov    sp, x0
    B      OsMainSchedule
loop1:
    B      loop1

/*
 * 描述: hwi分发
 */
    .globl OsHwiDispatcher
    .type OsHwiDispatcher, @function
    .align 4
OsHwiDispatcher:
    GENERAL_REGS_SAVE

    mrs    x3, spsr_el1
    mrs    x2, elr_el1
    sub    sp, sp, #16  // 跳过esr_el1, far_el1, 异常时才有用
    stp    x2, x3, [sp,#-16]!

    MOV    x0, SP

    BL     OsHwiDispatch   // 跳转C代码继续处理中断

    // 中断嵌套,tick嵌套场景, 会从OsHwiDispatch返回
    b     OsContextLoad

/*
 * 描述: void OsTskContextLoad(uintptr_t stackPointer)
 */
    .globl OsTskContextLoad
    .type OsTskContextLoad, @function
    .align 4
OsTskContextLoad:
    ldr    X0, [X0]
    mov    SP, X0            // X0 is stackPointer

OsContextLoad:
#ifdef OS_GDB_STUB
    bl      OsGdbMarkStep
#endif
    ldp    x2, x3, [sp],#16
    add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
    msr    spsr_el1, x3
    msr    elr_el1, x2
    dsb    sy
    isb
    ldp    xzr, x30, [sp],#16
    ldp    x29, x28, [sp],#16
    ldp    x27, x26, [sp],#16
    ldp    x25, x24, [sp],#16
    ldp    x23, x22, [sp],#16
    ldp    x21, x20, [sp],#16
    ldp    x19, x18, [sp],#16
    ldp    x17, x16, [sp],#16
    ldp    x15, x14, [sp],#16
    ldp    x13, x12, [sp],#16
    ldp    x11, x10, [sp],#16
    ldp    x9, x8, [sp],#16
    ldp    x7, x6, [sp],#16
    ldp    x5, x4, [sp],#16
    ldp    x3, x2, [sp],#16
    ldp    x1, x0, [sp],#16
    eret

    .globl OsSetSysStackSP
    .type OsSetSysStackSP, @function
    .align 4
OsSetSysStackSP:
    MOV    SP, X0
    MOV    X0, X1
    BL     OsHwiDispatchHandle

    .text
