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
@ * Description: exc汇编文件
@ */
    .align 8

    .section .text, "ax"
    .thumb
    .syntax unified

    .global  OsExcNmi
    .global  OsExcHardFault
    .global  OsExcMemFault
    .global  OsExcBusFault
    .global  OsExcUsageFault
    .global  OsExcSvcCall

    .type  OsExcNmi, function
    .type  OsExcHardFault, function
    .type  OsExcMemFault, function
    .type  OsExcBusFault, function
    .type  OsExcUsageFault, function
    .type  OsExcSvcCall, function

    .extern OsExcHandleEntryM4
    .extern g_uniFlag
    .extern g_curNestCount

OS_FLG_BGD_ACTIVE           = 0x0002
OS_FLG_MSP_ACTIVE           = 0x001D

@exc return using floating-point state flag
OS_FPU_SAVE_FLAG            = 0x10

@hardware push SP len
OS_FPU_PUSH_SP_AUTO         = 104 @auto save 18 float registers(S0~S15,CPSR,REV,SP 8bytes align) add 8 normal R registers,(18 + 8) * 4
OS_NORMAL_PUSH_SP_AUTO      = 32  @auto save 8 normal R registers(xPSR, PC, LR, R12,R0~R3),8*4

@exception type
NO_BMU_FAULT    = 0
BF_STKERR       = 1
BF_UNSTKERR     = 2
BF_IMPRECISERR  = 3
BF_PRECISERR    = 4
BF_IBUSERR      = 5

MF_MSTKERR      = 6
MF_MUNSTKERR    = 7
MF_DACCVIOL     = 8
MF_IACCVIOL     = 9

UF_DIVBYZERO    = 10
UF_UNALIGNED    = 11
UF_NOCP         = 12
UF_INVPC        = 13
UF_INVSTATE     = 14
UF_UNDEFINSTR   = 15

OS_EXC_CAUSE_NMI            = 16
OS_EXC_CAUSE_HARDFAULT      = 17

HF_DBGEVT                   = 19
HF_VECTBL                   = 20

FLAG_ADDR_VALID             = 0x10000

@if bit2 = 1,then ret use psp
OS_FALG_EXC_RET_PSP_STATUS  = 0x4

@UsageFault:0x030F; BusFault:0x1F(clear BFARVALID bit); MemFault:0x1B(clear MMARVALID bit)
OS_BMU_FAULT_CLEAR_BIT      = 0x030F1F1B

@Fault Reg valid exception bit
OS_BMU_FAULT_VALID_BIT      = 0x030F9F9B
OS_HARD_FAULT_VAILD_BIT     = 0xC0000002

OS_NVIC_FSR                 = 0xE000ED28      @include BusFault/MemFault/UsageFault State Regeister
OS_NVIC_HFSR                = 0xE000ED2C      @HardFault State Regeister
OS_NVIC_BFAR                = 0xE000ED38
OS_NVIC_MMAR                = 0xE000ED34
OS_NVIC_ACT_BASE            = 0xE000E300

excTbl:
    .byte 0,0,0,0,0,0,UF_DIVBYZERO,UF_UNALIGNED
    .byte 0,0,0,0,UF_NOCP,UF_INVPC,UF_INVSTATE,UF_UNDEFINSTR
    .byte 0,0,0,BF_STKERR,BF_UNSTKERR,BF_IMPRECISERR,BF_PRECISERR,BF_IBUSERR
    .byte 0,0,0,MF_MSTKERR,MF_MUNSTKERR,0,MF_DACCVIOL,MF_IACCVIOL
    .byte NO_BMU_FAULT,0,0,0

OsExcNmi:
    MOV     R0, #OS_EXC_CAUSE_NMI
    MOV     R1, #0
    B       OsExcDispatch

OsExcHardFault:
    MOV     R0, #OS_EXC_CAUSE_HARDFAULT
    LDR     R2, =OS_NVIC_HFSR
    LDR     R2, [R2]

    MOV     R1, #HF_DBGEVT
    LSL     R1, R1, #0x8
    ORR     R0, R1
    TST     R2, #0x80000000
    BNE     OsExcDispatch @ DBGEVT

    AND     R0, #0x000000FF
    MOV     R1, #HF_VECTBL
    LSL     R1, R1, #0x8
    ORR     R0, R1
    TST     R2, #0x00000002 @ bit1 indicates the fault of VECTBL
    BNE     OsExcDispatch   @ VECTBL

    @if not DBGEVT and VECTBL then is FORCED
    AND     R0, #0x000000FF

    LDR     R2, =OS_NVIC_FSR
    LDR     R2, [R2]

    TST     R2, #0x8000     @ BFARVALID
    BNE     _HFBusFault     @ BusFault

    TST     R2, #0x80       @ MMARVALID
    BNE     _HFMemFault     @ MemFault

    MOV     R12,#0
    B       OsHFExcCommonBMU

_HFBusFault:
    LDR     R1, =OS_NVIC_BFAR
    LDR     R1, [R1]
    MOV     R12, #FLAG_ADDR_VALID
    B       OsHFExcCommonBMU

_HFMemFault:
    LDR     R1, =OS_NVIC_MMAR
    LDR     R1, [R1]
    MOV     R12, #FLAG_ADDR_VALID

OsHFExcCommonBMU:
    LDR     R3, =OS_BMU_FAULT_CLEAR_BIT
    AND     R2, R3
    CLZ     R2, R2        @ NO_BMU_FAULT,when hard fault happen, no BMU fault(CFSR(R2) = 0, CLZ  R2, R2 -> R2 = 32)
    LDR     R3, =excTbl
    ADD     R3, R3, R2
    LDRB    R2, [R3]
    LSL     R2, R2, #0x8  @ORR  R0, R2, LSL #0x8
    ORR     R0, R2
    ORR     R0, R12
    B       OsExcDispatch

OsExcSvcCall:
    TST     LR, #0x4 @exc_return bit2
    MRSEQ   R0, MSP  @msp->R0
    MRSNE   R0, PSP  @psp->R0
    LDR     R1, [R0,#24]  @exc pc->R1
    LDRB    R0, [R1,#-2]  @get para0 from instrution SVC
    MOV     R1, #0
    B       OsExcDispatch

OsExcBusFault:
    LDR     R0, =OS_NVIC_FSR
    LDR     R0, [R0]

    TST     R0, #0x8000 @ BFARVALID
    BEQ     _ExcBusNoADDR
    LDR     R1, =OS_NVIC_BFAR
    LDR     R1, [R1]
    MOV     R12, #FLAG_ADDR_VALID

    B       OsExcCommonBMU

_ExcBusNoADDR:
    MOV     R12,#0
    B       OsExcCommonBMU

OsExcMemFault:
    LDR     R0, =OS_NVIC_FSR
    LDR     R0, [R0]

    TST     R0, #0x80 @ MMARVALID
    BEQ     _ExcMemNoADDR
    LDR     R1, =OS_NVIC_MMAR
    LDR     R1, [R1]
    MOV     R12, #FLAG_ADDR_VALID

    B       OsExcCommonBMU

_ExcMemNoADDR:
    MOV     R12,#0
    B       OsExcCommonBMU

OsExcUsageFault:
    LDR     R0, =OS_NVIC_FSR
    LDR     R0, [R0]

    @clear UsageFault invalid bit
    MOV     R12, #0

OsExcCommonBMU:
    LDR     R3, =OS_BMU_FAULT_CLEAR_BIT
    AND     R0, R3
    CLZ     R0, R0
    LDR     R3, =excTbl
    ADD     R3, R3, R0
    LDRB    R0, [R3]
    ORR     R0, R0, R12

@ R0 -- EXCCAUSE(bit 16 is 1 if EXCADDR valid),  R1 -- EXCADDR
OsExcDispatch:
    @ Clear Exeption status Reg
    LDR     R2, =OS_NVIC_FSR
    LDR     R3, =OS_BMU_FAULT_VALID_BIT
    STR     R3, [R2]

    @Clear Hardfault status Reg
    LDR     R2, =OS_NVIC_HFSR
    LDR     R3, =OS_HARD_FAULT_VAILD_BIT
    STR     R3, [R2]

    @ exc occured in Task or Init ,interrupt, or exc
    @ reserved for register info from task stack
    MOV     R2, R14                         @MSP:LR bit2 is 0; PSP:LR bit2 is 1
    TST     R2, #OS_FALG_EXC_RET_PSP_STATUS
    BEQ     _ExcInMSP                      @ exc occured in MSP stack
    B       _ExcInPSP                        @ exc occured in PSP stack

_ExcInMSP:
    ADD     R3, R13, #OS_NORMAL_PUSH_SP_AUTO  @ xPSR, PC, LR, R12,R0~R3 hardware save,8*4 bytes
    TST     LR, #OS_FPU_SAVE_FLAG             @ Is the MSP using the floating-point state?
    BNE     _MspStoreExcReg
    ADD     R3, R13, #OS_FPU_PUSH_SP_AUTO @ xPSR, PC, LR, R12,R0~R3 and float register hardware save

_MspStoreExcReg:
    PUSH    {R3}                          @ store message-->exc: MSP(R13),save IRQ SP
    MRS     R12, BASEPRI                  @ store message-->exc: disable int?
    PUSH    {R4-R12}                       @ store message-->exc: {R4-R12}

    B       _handleEntry

_ExcInPSP:
    @ exc occured in Task
    MOV     R2,  R13
    SUB     R13, #OS_NORMAL_PUSH_SP_AUTO      @ first add 8*4 Bytes Revs (for Reg. STMFD xPSR, PC, LR, R12,R0~R3)

    MRS     R3,  PSP                          @ get psp

    ADD     R12, R3, #OS_NORMAL_PUSH_SP_AUTO  @ xPSR, PC, LR, R12,R0~R3 hardware save,8*4 bytes
    TST     LR, #OS_FPU_SAVE_FLAG             @ Is the PSP using the floating-point state?
    BNE     _PspStoreExcReg
    ADD     R12, R3, #OS_FPU_PUSH_SP_AUTO     @ xPSR, PC, LR, R12,R0~R3 and float register hardware save

_PspStoreExcReg:
    PUSH    {R12}                         @ save task SP

    MRS     R12, BASEPRI
    PUSH    {R4-R12}                       @ save R4~R11,BASEPRI to MSP

    @ copy auto saved task register(xPSR, PC, LR, R12,R0~R3)from psp stack
    LDMFD   R3, {R4-R11}                  @ R4-R11 store PSP reg(auto push xPSR, PC, LR, R12,R0~R3 when exc in task)
    STMFD   R2!, {R4-R11}

_handleEntry:
    MOV     R2, R13                         @ R13:the 3th param
    B       OsExcHandleEntryM4

    NOP
    .align
    .end
