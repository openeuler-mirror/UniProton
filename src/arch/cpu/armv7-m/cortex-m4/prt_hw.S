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
    .align 8
    .section .text, "ax"
    .thumb
    .syntax unified

    .global  OsAsmIll
    .global  OsGetSp
    .global  OsFpuInit

    .type   OsAsmIll, function
    .type   OsGetSp, function
    .type   OsFpuInit,function

OS_EXC_CAUSE_FATAL_ERR       =   18

OsAsmIll:
    SVC     OS_EXC_CAUSE_FATAL_ERR
LOOP:
    B       LOOP

OsGetSp:
    MRS     R0, PSP          @ 读取PSP
    BX      LR

OsFpuInit:
    LDR     R0, = 0xE000ED88 @ Enable CP10, CP11
    LDR     R1, [R0]
    ORR     R1, R1, #(0xF << 20)
    STR     R1, [R0]

    LDR     R0, = 0xE000EF34 @ FPU is automatic save or store mode
    LDR     R1, [R0]
    AND     R1, R1, #0XBFFFFFFF
    ORR     R1, R1, #0X80000000
    STR     R1, [R0]
    BX      LR

    .align

    .end
