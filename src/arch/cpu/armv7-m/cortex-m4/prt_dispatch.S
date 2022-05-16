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
@ * Create: 2009-07-24
@ * Description: thread scheduler
@ */
    .align 8

    .global  PRT_HwiLock
    .global  PRT_HwiUnLock
    .global  PRT_HwiRestore
    .global  OsFirstTimeSwitch
    .global  OsTaskSwitch
    .global  OsHwiSwitch
    .global  OsPendSv
    .global  OsIntNumGet
    .global  OsSvchandler
    .global  OsRestoreGeneralR
    .global  OsTaskSwtichHook
    .global  OsViSwitchRet

    .type PRT_HwiLock, function
    .type PRT_HwiUnLock, function
    .type PRT_HwiRestore, function
    .type OsFirstTimeSwitch, function
    .type FirstTaskSwtich,function

    .type OsIntNumGet, function
    .type OsTaskSwitch, function
    .type OsHwiSwitch,function
    .type OsSvchandler, function
    .type OsPendSv, function
    .type OsGoViDispatch,function
    .type OsViSwitchRet, function
    .type OsVi2Task,function
    .type OsTaskContexSave,function
    .type OsRestoreGeneralR, function
    .type OsTaskSwtich,function
    .type OsTaskLoad,function
    .type OsNotLoadFloat,function
    .type OsTaskEnd,function
    .type OsTaskSwtichHook,function
    .type OsViDispatch,function

    .extern  g_runningTask
    .extern  g_highestTask
    .extern  g_stackEnd
    .extern  g_uniFlag
    .extern  OsViDispatch
    .extern  g_tickNoRespondCnt
    .extern  g_excTrap
    .extern  OsTskSwitchHookCaller

OS_NVIC_INT_CTRL           = 0xE000ED04
OS_NVIC_SYSPRI2            = 0xE000ED20
OS_NVIC_PENDSV_PRI         = 0xF0F00000
OS_NVIC_PENDSVSET          = 0x10000000
OS_TSK_RUNNING             = 0x0080

OS_INT_HIGHEST_LEVEL       = 0x80
OS_INT_LOWEST_LEVEL        = 0x00

OS_EXC_RETURN_TM_MSP       = 0xFFFFFFF9

@exc return flag
OS_FPU_SAVE_FLAG           = 0x10
OS_SP_SELECT_FLAG          = 0x04

@hardware push SP len
OS_FPU_PUSH_SP_AUTO        = 104
OS_NORMAL_PUSH_SP_AUTO     = 32

@switch flag
OS_SVC_TSK_SWICH           = 1
OS_SVC_VI2TASK             = 2

OS_FLG_BGD_ACTIVE          = 0x0002

OS_FLG_SYS_ACTIVE          = 0x0010

OS_FLG_TSK_REQ             = 0x1000

OS_FLG_TSK_SWHK            = 0x2000

OS_VI_XPSR                 = 0x01000000

    .section .text, "ax"
    .thumb
    .syntax unified

OsFirstTimeSwitch:
    LDR     R4, =OS_NVIC_SYSPRI2
    LDR     R5, =OS_NVIC_PENDSV_PRI
    STR     R5, [R4]

    @ reset MSP
    LDR     R0, =g_stackEnd
    MSR     MSP, R0

FirstTaskSwtich:
    @use PSP in thread mode from now
    MOV     R0, #2
    MSR     CONTROL, R0

    @g_uniFlag
    LDR     R0, =g_uniFlag
    LDR     R1, [R0]
    ORR     R1, R1,#OS_FLG_BGD_ACTIVE

    STR     R1, [R0]

    @g_runningTask = g_highestTask
    LDR     R0, =g_highestTask
    LDR     R0, [R0]
    LDR     R1, =g_runningTask
    STR     R0, [R1]

    @g_runningTask->taskStatus |= OS_TSK_RUNNING
    LDRH    R1, [R0, #4]
    ORR     R1, R1, #OS_TSK_RUNNING
    STRH    R1, [R0, #4]

    @ get func parameter from stack
    LDR     R1, [R0]
    ADD     R1, R1, #40
    LDR     R5, [R1]

    @get LR,PC,XPSR from stack
    ADD     R1, R1, #20
    LDMFD   R1!, {R2-R4}
    MSR     PSP, R1
    MOV     LR, R2
    MSR     xPSR, R4
    MOV     R0, R5
    CPSIE   I

    BX      R3

    .align

    .section .kernel, "ax"
    .thumb
    .syntax unified

PRT_HwiLock:
    MRS     R0, BASEPRI
    MOV     R1, #OS_INT_HIGHEST_LEVEL
    MSR     BASEPRI, R1
    BX      LR

PRT_HwiUnLock:
    MRS     R0, BASEPRI
    MOV     R1, #OS_INT_LOWEST_LEVEL
    MSR     BASEPRI, R1
    BX      LR

PRT_HwiRestore:
    MSR     BASEPRI, R0
    BX      LR

OsIntNumGet:
    MRS     R0, IPSR
    BX      LR

OsTaskSwitch:
    SVC     #OS_SVC_TSK_SWICH
    BX      LR

OsHwiSwitch:
    LDR     R0, =OS_NVIC_INT_CTRL
    LDR     R1, =OS_NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR

OsSvchandler:
    TST     LR, #OS_SP_SELECT_FLAG
    ITE     EQ
    MRSEQ   R0, MSP
    MRSNE   R0, PSP

    @get pc
    LDR     R1, [R0,#24]

    @get svc instruction
    LDRB    R0, [R1,#-2]

    CMP     R0, #OS_SVC_TSK_SWICH
    BEQ     OsPendSv

    CMP     R0, #OS_SVC_VI2TASK
    BEQ     OsVi2Task

    @ excpetion
    LDR     R1, =g_excTrap
    LDR     R1, [R1]
    CMP     R1, #0
    BXNE    R1
    B .

OsPendSv:
    MRS     R12, BASEPRI
    MOV     R2, #OS_INT_HIGHEST_LEVEL
    MSR     BASEPRI, R2

    @tick switch
    LDR     R0, =g_tickNoRespondCnt
    LDR     R3, [R0]
    CMP     R3, #0
    BNE     OsGoViDispatch

    @task switch
    B       OsTaskContexSave

OsGoViDispatch:
    push    {R12,LR}

    @call tick dispatch
    LDR     R3, =OS_VI_XPSR
    LDR     R2, =OsViDispatch
    LDR     R1, =OsViSwitchRet
    push    {R1-R3}
    sub     sp,sp,#20

    @thread mode msp
    MOV     LR, #OS_EXC_RETURN_TM_MSP
    BX      LR

OsViSwitchRet:
    SVC     #OS_SVC_VI2TASK
    B .

OsVi2Task:
    TST     LR, #OS_FPU_SAVE_FLAG
    ITE     EQ
    MOVEQ   R2, #OS_FPU_PUSH_SP_AUTO
    MOVNE   R2, #OS_NORMAL_PUSH_SP_AUTO
    ADD     SP, SP, R2
    pop     {R12, LR}

OsTaskContexSave:
    TST     LR, #OS_SP_SELECT_FLAG
    BEQ     OsTaskEnd

    @save task context
    LDR     R0, =g_uniFlag
    LDR     R1, [R0]
    MOV     R3, R1

    BIC     R1, R1, #OS_FLG_TSK_REQ /* g_uniFlag &= ~OS_FLG_TSK_REQ */
    STR     R1, [R0]
    TST     R3, #OS_FLG_TSK_REQ
    BEQ     OsTaskEnd

    @get PSP
    MRS     R0, PSP

    @Is the task using the FPU context? If so, push high vfp registers.
    TST     LR, #OS_FPU_SAVE_FLAG
    BNE     OsRestoreGeneralR
    @store s16-s31
    VSTMDB  R0!, {S16-S31}

OsRestoreGeneralR:
    @store R4-R11,BASEPRI,EXC_RETURN
    STMFD   R0!, {R4-R12, LR}

    @g_runningTask->stackPointer = PSP
    LDR     R5, =g_runningTask
    LDR     R6, [R5]
    STR     R0, [R6]

    @g_runningTask->taskStatus &= ~OS_TSK_RUNNING
    LDRH    R1, [R6, #4]
    BIC     R1, R1, #OS_TSK_RUNNING
    STRH    R1, [R6, #4]

    @get new task
    LDR     R7, =g_highestTask
    LDR     R7, [R7]

OsTaskSwtich:
    @g_runningTask = g_highestTask
    STR     R7, [R5]

    @g_runningTask->taskStatus |= OS_TSK_RUNNING
    LDRH    R1, [R7, #4]
    ORR     R1, R1, #OS_TSK_RUNNING
    STRH    R1, [R7, #4]

    @task switch hook,dot not change R5 R6 R7
    LDR     R0, =g_uniFlag
    LDR     R1, [R0]
    TST     R1, #OS_FLG_TSK_SWHK
    BNE     OsTaskSwtichHook

OsTaskLoad:
    @load R4-R11,BASEPRI,EXC_RETURN
    LDR     R1, [R7]
    LDMFD   R1!, {R4-R12,LR}

    TST     LR, #OS_FPU_SAVE_FLAG
    BNE     OsNotLoadFloat
    @load s16-s31
    VLDMIA  R1!, {S16-S31}

OsNotLoadFloat:
    MSR     PSP, R1

OsTaskEnd:
    MSR     BASEPRI, R12
    @auto restore R0-R3,R12,lr,pc,xpsr
    BX      LR

OsTaskSwtichHook:
    LDR     R3, =OsTskSwitchHookCaller
    LDR     R0, [R6, #16]
    LDR     R1, [R7, #16]
    LDR     LR, =OsTaskLoad
    BX      R3
    .align
    .end
