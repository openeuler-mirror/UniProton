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
 * Description: GUEST启动处理。
 */
#include "prt_buildef.h"
#include "prt_asm_arm_external.h"

DAIF_MASK = 0x1C0       // disable Abort, IRQ, FIQ

.macro REG_INIT  xreg
    mov \xreg, #0
.endm


    .global  __os_sys_sp_end
    .global  g_cfgPrimaryCore
    .global  g_gicCoreMap
    .global  OsResetVector
    .global  OsBssInit
    .global  InitSystemSp
    .global  OsVectorTable

    .type   __os_sys_sp_end, object
    .type   g_cfgPrimaryCore, object
    .type   g_gicCoreMap, object
    .type   OsResetVector, function
    .type   OsBssInit, function

    .section .text.startup, "ax"
OsResetVector:
OsVectTblInit:
    LDR x0, =OsVectorTable
    MSR VBAR_EL1, X0

OsGeneralRegsInit:  // The X30 general-purpose register is used as the procedure call link register
    REG_INIT x0
    REG_INIT x1
    REG_INIT x2
    REG_INIT x3
    REG_INIT x4
    REG_INIT x5
    REG_INIT x6
    REG_INIT x7
    REG_INIT x8
    REG_INIT x9
    REG_INIT x10
    REG_INIT x11
    REG_INIT x12
    REG_INIT x13
    REG_INIT x14
    REG_INIT x15
    REG_INIT x16
    REG_INIT x17
    REG_INIT x18
    REG_INIT x19
    REG_INIT x20
    REG_INIT x21
    REG_INIT x22
    REG_INIT x23
    REG_INIT x24
    REG_INIT x25
    REG_INIT x26
    REG_INIT x27
    REG_INIT x28
    REG_INIT x29

#if defined(OS_OPTION_SMP)
OsStackBssInit:
    OsAsmGetCoreId  x0 // 读取核号
    LDR x4, =g_cfgPrimaryCore
    LDR w4, [x4]
    CMP w0, w4
    BNE OsSecondaryStackInit

    LDR x0, =__os_sys_sp_end
    BIC sp, x0, #0xf                /* 16-byte alignment for ABI compliance */
    BL  OsBssInit
    BL  InitSystemSp

    B   OsCoreMapInit

OsSecondaryStackInit:
    LDR x1, =g_sysStackHigh
    LDR x4, [x1,w0,uxtw #3]

    CMP x4, #0
    BEQ OsSecondaryStackInit
    MOV SP, x4

OsCoreMapInit:
    MRS x1, mpidr_el1 // 读取CoreMap
    LDR x2, =g_gicCoreMap
    OsAsmGetCoreId  x3 // 读取核号
    /* g_gicCoreMap[coreid] 赋值对应coreid的mpidr_el1 */
    STR x1, [x2, x3, lsl #3]

#else
OsStackBssInit:
    LDR x0, =__os_sys_sp_end
    BIC sp, x0, #0xf                /* 16-byte alignment for ABI compliance */
    BL  OsBssInit
    BL  InitSystemSp
    B   OsCoreMapInit

OsCoreMapInit:
    MRS x1, mpidr_el1 // 读取CoreMap
    LDR x2, =g_gicCoreMap
    STR x1, [x2]

#endif

#if defined(OS_OPTION_SMP)
OsPreMain:
    LDR x0, =g_cfgPrimaryCore
    LDR w4, [x0]

    CMP w3, w4
    BEQ OsPrimaryMain
    /* 从核能否进入main标志，只有主核完成了资源的注册核初始化，ResetFlag才会被置位 */
    LDR x0, =g_secondaryResetFlag

polling: /* 从核启动，循环等待入口 */
    LDR w2, [x0]
    CMP w2, #0 /* 判断g_secondaryResetFlag是否置位 */
    BNE OsEnterMain /* 置位则从核跳转至主函数。do{} while(); try at least once */
    WFE
    B polling
#endif

OsPrimaryMain: /* OsPrimaryMain函数主核调用 */
    BL PRT_HardBootInit /* 用户调用的第一个函数钩子，用户在此处完成Seed随机数生成 */
    /* Notes：X9存g_memCanaryRdm地址，X10存__stack_chk_guard地址，w11存g_memCanaryRdm的值 U32类型 */
    /* __stack_chk_guard赋值后 g_memCanaryRdm 以及 w11都会被销毁 */
#if defined(OS_OPTION_GUARD_STACK)
    InitChkGuardRnd x9, x10, w11 /* RND 写入 StackChkGuard */
#endif
#ifdef OS_GDB_STUB
    bl OsGdbStubInit
#endif

OsEnterMain:
    BL      main

    MOV     x2, DAIF_MASK
    MSR     DAIF, x2

EXITLOOP:
    B EXITLOOP

    .text
