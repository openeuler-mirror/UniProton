@/*
@ * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
@ *
@ * UniProton is licensed under Mulan PSL v2.
@ * You can use this software according to the terms and conditions of the Mulan PSL v2.
@ * You may obtain a copy of Mulan PSL v2 at:
@ *          http://license.coscl.org.cn/MulanPSL2
@ * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
@ * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
@ * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
@ * See the Mulan PSL v2 for more details.
@ * Create: 2009-12-22
@ * Description: thread scheduler
@ */
#include "prt_buildef.h"
#include "prt_asm_arm_external.h"

    .align 8
    .section .reset,"ax"
    .thumb
    .syntax unified

    .extern  g_stackEnd
    .extern  g_stackStart
    .extern  g_bssStart
    .extern  g_bssEnd
    .extern  g_hwiTable
    .extern __clibrary_init

@init import exc vector table
    .extern  OsExcMemFault
    .extern  OsExcBusFault
    .extern  OsExcUsageFault
    .extern  OsExcSvcCall
    .extern  main
    .extern  PRT_HardBootInit

    .global  g_bootVectors
    .global  OsResetVector
    .global  OsFpuInit

    .type g_bootVectors,function
    .type OsResetVector,function
    .type OsFpuInit,function

OS_NVIC_SHCSR                       =   0xE000ED24
OS_NVIC_CCR                         =   0xE000ED14

@Vector table offset addr
OS_NVIC_VTOR                        =   0xE000ED08
@Enable USGFAULT, BUSFAULT, MEMFAULT
OS_NVIC_UBM_FAULT_ENABLE            =   0x70000
@Enable DIV 0, but disbale unaligned exception, because filesystem have unaligned operation
OS_NVIC_UBM_DIV_0_TRP_ENABLE        =   0x10

g_bootVectors:
    .long     g_stackEnd
    .long     OsResetVector
    .long     OsBootNMI               @system boot processs
    .long     OsBootHardFault         @system boot processs
    .long     OsExcMemFault
    .long     OsBootNMI
    .long     OsExcBusFault
    .long     0
    .long     0
    .long     0
    .long     0
    .long     OsExcSvcCall

@ Reset handler
OsResetVector :
    @disable interrupt excluding NMI and HardFault
    CPSID   I

    @init exception
    @init vector table offset reg
    LDR     R1, =g_stackEnd
    MSR     MSP, R1
    LDR     R0, =g_bootVectors
    LDR     R1, =OS_NVIC_VTOR
    STR     R0, [R1]

    @Enable exception including USGFAULT, BUSFAULT, MEMFAULT
    @*(volatile U32 *)OS_NVIC_SHCSR |= 0x70000
    LDR     R0, =OS_NVIC_SHCSR
    LDR     R1, [R0]
    ORR     R1, #OS_NVIC_UBM_FAULT_ENABLE
    STR     R1, [R0]

    @Enable DIV 0 excetion
    @*(volatile U32 *)OS_NVIC_CCR |= 0x10@
    LDR     R0, =OS_NVIC_CCR
    LDR     R1, [R0]
    ORR     R1, #OS_NVIC_UBM_DIV_0_TRP_ENABLE
    STR     R1, [R0]

    @modify MSP to 0xCACACACA
    LDR     R0, =g_stackEnd  @top of stack
    LDR     R1, =g_stackStart   @bottom of stack
    SUB     R0, R0, R1
    CMP     R0, #4          @stack length <4?
    BLT     LOOP2

LOOP1:                      @the condition of stack length >=4
    MOV     R2, #0xCACACACA
    STR     R2, [R1]
    ADD     R1, R1, #4
    SUB     R0, R0, #4
    CMP     R0, #4
    BGE     LOOP1
    CMP     R0, #0          @four bytes align?
    BEQ     BSS_INIT

LOOP2:                      @the condition of stack length <4
    MOV     R2, #0xCA
    STRB    R2, [R1]
    ADD     R1, R1, #1
    SUB     R0, R0, #1
    CMP     R0, #0
    BNE     LOOP2

BSS_INIT:
    @modify BSS to 0x0
    LDR     R0, =g_bssEnd
    LDR     R1, =g_bssStart
    SUB     R0, R0, R1
    CMP     R0, #4          @bss length <4?
    BLT     BSS_INIT_LOOP2

BSS_INIT_LOOP1:             @the condition of bss length >=4
    MOV     R2, #0x0
    STR     R2, [R1]
    ADD     R1, R1, #4
    SUB     R0, R0, #4
    CMP     R0, #4
    BGE     BSS_INIT_LOOP1
    CMP     R0, #0          @four bytes align?
    BEQ     OsBoot

BSS_INIT_LOOP2:             @the condition of bss length <4
    MOV     R2, #0x0
    STRB    R2, [R1]
    ADD     R1, R1, #1
    SUB     R0, R0, #1
    CMP     R0, #0
    BNE     BSS_INIT_LOOP2

OsBoot:
    BL      OsFpuInit
    LDR     R0, =PRT_HardBootInit
    BLX     R0
#if defined(OS_OPTION_GUARD_STACK)
    InitChkGuardRnd  R9, R10, R11
#endif
    LDR     R0, =main
    BX      R0

OsBootNMI:
    B .

OsBootHardFault:
    B .

@*********************************************************************************************************
@*  The function expected of the C library startup
@*  code for defining the stack and heap memory locations.
@*********************************************************************************************************
@        ALIGN
@        AREA    |.text|, CODE, READONLY
@
@        IMPORT  __use_two_region_memory
@        EXPORT  __user_initial_stackheap
@
@__user_initial_stackheap
@        LDR     R0, =HeapMem
@        LDR     R1, =(StackMem + Stack)
@        LDR     R2, =(HeapMem + Heap)
@        LDR     R3, =StackMem
@        BX      LR

    .align
    .end
