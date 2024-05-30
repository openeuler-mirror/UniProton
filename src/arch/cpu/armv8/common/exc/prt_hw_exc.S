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
 * Description: 异常处理的汇编部分。
 */

#include "prt_asm_cpu_external.h"

    .global OsExcHandleEntry
    .type   OsExcHandleEntry, function
    .equ OS_TSKCONTEXT_SIZE, 0x120

// save x2~x30, xzr
.macro GENERAL_REGS_SAVE
    stp    x3, x2, [sp,#-16]!
    stp    x5, x4, [sp,#-16]!
    stp    x7, x6, [sp,#-16]!
    stp    x9, x8, [sp,#-16]!
    stp    x11, x10, [sp,#-16]!
    stp    x13, x12, [sp,#-16]!
    stp    x15, x14, [sp,#-16]!
    stp    x17, x16, [sp,#-16]!
    stp    x19, x18, [sp,#-16]!
    stp    x21, x20, [sp,#-16]!
    stp    x23, x22, [sp,#-16]!
    stp    x25, x24, [sp,#-16]!
    stp    x27, x26, [sp,#-16]!
    stp    x29, x28, [sp,#-16]!
    stp    xzr, x30, [sp,#-16]!
.endm

.macro ELX_REGS_INIT
    mov    x10, xzr
    mov    x11, xzr
    mov    x12, xzr
    mov    x13, xzr
    mov    x14, xzr
    mov    x15, xzr
    mov    x16, xzr
    mov    x17, xzr
    mov    x18, xzr
    mov    x19, xzr
.endm

.section .os.init.text, "ax"

    .globl OsSaveElxRegs
    .type OsSaveElxRegs, @function
    .align 4
OsSaveElxRegs:
    stp    x18, x19, [sp,#-16]!
    stp    x16, x17, [sp,#-16]!
    stp    x14, x15, [sp,#-16]!
    stp    x12, x13, [sp,#-16]!
    stp    x10, x11, [sp,#-16]!
    ret

    .globl OsExcDispatch
    .type OsExcDispatch, @function
    .align 4
OsExcDispatch:
    GENERAL_REGS_SAVE

    mrs    x5, esr_el1
    mrs    x4, far_el1
    mrs    x3, spsr_el1
    mrs    x2, elr_el1
    stp    x4, x5, [sp,#-16]!
    stp    x2, x3, [sp,#-16]!

    mov    x0, sp

    /* (X19-X29) These registers are saved in the callee frame.  */
    mov    x22, x1 // x22: save excType
    /* 异常后又压栈了TskContext大小 */
    /* 记录异常发生现场时需要把这部分退栈回去 */
    add    x21, sp, #OS_TSKCONTEXT_SIZE // x21: callee-save, x21=original sp
    BL     OsSwitchToSysStack
    mov    sp, x0

    mrs    x20, CurrentEL
    mrs    x19, vbar_el1
    mrs    x18, sctlr_el1
    mrs    x17, mair_el1
    mrs    x16, tcr_el1
    mrs    x15, ttbr1_el1
    mrs    x14, ttbr0_el1
    stp    x20, x21, [sp,#-16]!
    stp    x18, x19, [sp,#-16]!
    stp    x16, x17, [sp,#-16]!
    stp    x14, x15, [sp,#-16]!

    mov    w0, w22
    mov    x1, sp
    bl     OsExcHandleEntry

    sub    x21, x21, #OS_TSKCONTEXT_SIZE
    mov    sp, x21 // x21=original sp
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

    .text
