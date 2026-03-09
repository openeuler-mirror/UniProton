/*
 * Copyright (c) 2024, Greater Bay Area National Center of Technology Innovation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-06-14     WuPeifeng    the first version
 */

#include "armv7_r.h"

#define FUNCTION(x)             .global x; .type x,STT_FUNC; x:

.globl _start

.section ".text.boot"

_start:
    B   arm_reset
    B   _osExceptUndefInstrHdl
    B   _osExceptSwiHdl
    B   _osExceptPrefetchAbortHdl
    B   _osExceptDataAbortHdl
    B   _osExceptAddrAbortHdl
    B   OsHwiDispatcher
    B   _osExceptFiqHdl

FUNCTION(arm_reset)
OsStackBssInit:
    /* 初始化通用寄存器 */
    MOV R0, #0
    MOV R1, #0
    MOV R2, #0
    MOV R3, #0
    MOV R4, #0
    MOV R5, #0
    MOV R6, #0
    MOV R7, #0
    MOV R8, #0
    MOV R9, #0
    MOV R10, #0
    MOV R11, #0
    MOV R12, #0

    /* 进入svc模式，关闭IRQ,FIQ*/
    MOV     R0, #(PSR_SVC_MODE | PSR_I_BIT | PSR_F_BIT)
    MSR     cpsr_c, R0

    MRC     p15, 0, R12, c1, c0, 0
    BIC     R12, R12, #((1 << 0) | (1 << 1) | (1 << 2))
    BIC     R12, R12, #((1 << 10) | (1 << 12) | (1 << 14))
    BIC     R12, R12, #((1 << 25) | (1 << 30))
    ORR     R12, R12, #((1 << 11))
    BIC     R12, R12, #((1 << 13))
    MCR     p15, 0, R12, c1, c0, 0

    MRC     p15, 0, R0, c15, c0, 1
    TST     R0, #(0x1F << 2)
    BEQ     .Lno_normal_axi_pp

    ORR     R0, R0, #1
    MCR     p15, 0, R0, c15, c0, 1
.Lno_normal_axi_pp:
    MRC     p15, 0, R0, c15, c0, 2
    TST     R0, #(0x1F << 2)
    BEQ     .Lno_virtual_axi_pp

    ORR     R0, R0, #1
    MCR     p15, 0, R0, c15, c0, 2
.Lno_virtual_axi_pp:
    MRC     p15, 0, R0, c15, c0, 3
    TST     R0, #(0x1F << 2)
    BEQ     .Lno_ahb_pp

    ORR     R0, R0, #1
    MCR     p15, 0, R0, c15, c0, 3
.Lno_ahb_pp:

/* 初始化各个模式对应的私有寄存器和栈空间 */
.Lstack_setup:
    /* IRQ */
    CPSID   i, #PSR_IRQ_MODE
    MOV     LR, #0
    LDR     R12, =irq_stack
    ADD     R12, #OS_ARCH_IRQ_STACK_SIZE
    MOV     SP, R12

    /* FIQ */
    CPSID   i, #PSR_FIQ_MODE
    MOV     R8, #0
    MOV     R9, #0
    MOV     R10, #0
    MOV     R11, #0
    MOV     R12, #0
    MOV     SP, R0
    MOV     LR, #0
    LDR     R12, =fiq_stack
    ADD     R12, #OS_ARCH_FIQ_STACK_SIZE
    MOV     SP, R12

    /* data abort */
    CPSID   i, #PSR_ABT_MODE
    MOV     LR, #0
    LDR     R12, =abt_stack
    ADD     R12, #OS_ARCH_ABT_STACK_SIZE
    MOV     SP, R12

    /* undefined instruction */
    CPSID   i, #PSR_UNDEF_MODE
    MOV     LR, #0
    LDR     R12, =und_stack
    ADD     R12, #OS_ARCH_UND_STACK_SIZE
    MOV     SP, R12

    /* system/user 共用一个栈*/
    CPSID   i, #PSR_SYS_MODE
    MOV     LR, #0
    LDR     R12, =sys_stack
    ADD     R12, #OS_ARCH_SYS_STACK_SIZE
    MOV     SP, R12

    /* supervisor */
    CPSID   i, #PSR_SVC_MODE
    MOV     LR, #0
    LDR     R12, =svc_stack
    ADD     R12, #OS_ARCH_SVC_STACK_SIZE
    MOV     SP, R12


    /* 使能cache*/
    LDR     R0, =ICACHE
    BL      arch_enable_cache

    LDR     R0, =DCACHE
    BL      arch_enable_cache

.Lbss_init:
    LDR     R4, =__bss_start__
    LDR     R5, =__bss_end__
    MOV     R6, #0
.Lbss_loop:
    CMP     R4, R5
    STRLT   R6, [R4], #4
    BLT     .Lbss_loop

OsPrimaryMain: /* OsPrimaryMain函数主核调用 */
    BL PRT_HardBootInit /* 用户调用的第一个函数钩子，用户在此处完成Seed随机数生成 */
    LDR     R0, =main
    BLX     R0
    B       .
