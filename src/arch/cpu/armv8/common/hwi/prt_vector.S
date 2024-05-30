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
 * Description: 向量表处理。
 */
#include "prt_buildef.h"

.macro EXC_HANDLE vecId
    stp x1, x0, [sp,#-16]!
    mov x1, #\vecId
    b   OsExcDispatch
.endm

#ifdef OS_GDB_STUB
.section .os.init.text, "ax"
.extern OsGdbHandleException

.equ S_FRAME_SIZE, (34 * 8) // sizeof(struct prt_regs)
.equ S_LR, (30 * 8) // offset of struct prt_regs->x30
.equ S_PC, (32 * 8) // offset of struct prt_reg->pc
    .globl el1_dbg
    .type el1_dbg, @function
    .align 4

el1_dbg:
    sub sp, sp, #S_FRAME_SIZE

    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    add x21, sp, #S_FRAME_SIZE
    stp lr, x21, [sp, #S_LR]

    mrs x22, elr_el1
    mrs x23, spsr_el1
    stp x22, x23, [sp, #S_PC]

    mov x0, sp
    bl OsGdbHandleException

    msr daifset, 0xf

    ldp x22, x23, [sp, #S_PC]        // load ELR, SPSR
    msr elr_el1, x22            // set up the return data
    msr spsr_el1, x23

    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr lr, [sp, #S_LR]

    add sp, sp, #S_FRAME_SIZE        // restore sp

    dsb    sy
    isb

    eret

.macro el1_sync_handler vecId
    stp x1, x0, [sp, #-16]!

    mrs x0, esr_el1
    lsr x0, x0, #26
    and x0, x0, #0x3F // get ESR_EL1_EC

    cmp x0, #0x31 // EC_BREAKPT
    b.eq 1f
    cmp x0, #0x33 // EC_SOFTSTP
    b.eq 1f
    cmp x0, #0x35 // EC_WATCHPT
    b.eq 1f
    cmp x0, #0x3C // EC_BRK64
    b.eq 1f

    ldp x1, x0, [sp], #16
    EXC_HANDLE \vecId

1:
    ldp x1, x0, [sp], #16
    b el1_dbg
.endm
.text
#endif

    .section .os.vector.text, "ax"

    .global  OsVectorTable
    .global  OsHwiDispatcher
    .global  OsVectorTable
    .type   OsHwiDispatcher, function
    .type  OsVectorTable,function

    .align 13

OsVectorTable:
.set    VBAR, OsVectorTable
.org VBAR                                // Synchronous, Current EL with SP_EL0
    EXC_HANDLE  0

.org (VBAR + 0x80)                       // IRQ/vIRQ, Current EL with SP_EL0
    EXC_HANDLE  1

.org (VBAR + 0x100)                      // FIQ/vFIQ, Current EL with SP_EL0
    EXC_HANDLE  2

.org (VBAR + 0x180)                      // SERROR, Current EL with SP_EL0
    EXC_HANDLE  3

.org (VBAR + 0x200)                      // Synchronous, Current EL with SP_ELx
#ifdef OS_GDB_STUB
    el1_sync_handler 4
#else
    EXC_HANDLE  4
#endif

.org (VBAR + 0x280)                      // IRQ/vIRQ, Current EL with SP_ELx
    stp x1, x0, [sp,#-16]!
    mov x1, #5
    b   OsHwiDispatcher

.org (VBAR + 0x300)                      // FIQ/vFIQ, Current EL with SP_ELx
    EXC_HANDLE  6

.org (VBAR + 0x380)                      // SERROR, Current EL with SP_ELx
    EXC_HANDLE  7

.org (VBAR + 0x400)                      // Synchronous, EL changes and the target EL is using AArch64
    EXC_HANDLE  8

.org (VBAR + 0x480)                      // IRQ/vIRQ, EL changes and the target EL is using AArch64
    EXC_HANDLE  9

.org (VBAR + 0x500)                      // FIQ/vFIQ, EL changes and the target EL is using AArch64
    EXC_HANDLE  10
.org (VBAR + 0x580)                      // SERROR, EL changes and the target EL is using AArch64
    EXC_HANDLE  11
.org (VBAR + 0x600)                      // Synchronous, L changes and the target EL is using AArch32
    EXC_HANDLE  12

.org (VBAR + 0x680)                      // IRQ/vIRQ, EL changes and the target EL is using AArch32
    EXC_HANDLE  13

.org (VBAR + 0x700)                      // FIQ/vFIQ, EL changes and the target EL is using AArch32
    EXC_HANDLE  14
.org (VBAR + 0x780)                      // SERROR, EL changes and the target EL is using AArch32
    EXC_HANDLE  15

    .text

