/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Description: Aarch64 register context save/restore for low-power suspend/resume.
 *              Ported from LiteOs arch/arm64/src/runstop.S. The board runs at EL1,
 *              so SPSR_ELx/ELR_ELx map to spsr_el1/elr_el1 and RUNLVL is 4.
 */

/* EL1 mappings (sd3403 runs at EL1, see demos/sd3403 -DENV_EL1). */
#define RUNLVL     4
#define SPSR_ELx   spsr_el1
#define ELR_ELx    elr_el1

    .extern g_lowpowerSaveAR
    .extern g_lowpowerSaveSRContext

    .global OsLowpowerSaveRegister
    .global OsLowpowerRestoreRegister

    .arch armv8-a
    .text

/*
 * What we save:
 *   SP, LR, DAIF, NZCV, X0 - X29 [30]
 * OsLowpowerSaveRegister:
 *   Saves the current CPU context into g_lowpowerSaveSRContext, using
 *   g_lowpowerSaveAR to temporarily preserve X0/X1/X2 across the routine.
 */
OsLowpowerSaveRegister:
    STP X2, X3, [SP, #-16]!
    LDR X3, =g_lowpowerSaveAR
    STR X0, [X3]
    STR X1, [X3, #8]
    STR X2, [X3, #16]
    LDP X2, X3, [SP], #16

    LDR X0, =g_lowpowerSaveSRContext
    ADD X0, X0, #272

    /* PUSH {SP, LR} */
    MOV X1, SP
    STP X1, X30, [X0, #-16]!

    /* PUSH {DAIF, NZCV} */
    MRS X1, DAIF
    MRS X2, NZCV
    STP X1, X2, [X0, #-16]!

    STP X28, X29, [X0, #-16]!
    STP X26, X27, [X0, #-16]!
    STP X24, X25, [X0, #-16]!
    STP X22, X23, [X0, #-16]!
    STP X20, X21, [X0, #-16]!
    STP X18, X19, [X0, #-16]!
    STP X16, X17, [X0, #-16]!
    STP X14, X15, [X0, #-16]!
    STP X12, X13, [X0, #-16]!
    STP X10, X11, [X0, #-16]!
    STP X8, X9, [X0, #-16]!
    STP X6, X7, [X0, #-16]!
    STP X4, X5, [X0, #-16]!

    MOV X29, X0

    LDR X0, =g_lowpowerSaveAR
    LDR X0, [X0]
    LDR X1, =g_lowpowerSaveAR
    LDR X1, [X1, #8]
    LDR X2, =g_lowpowerSaveAR
    LDR X2, [X2, #16]

    STP X2, X3, [X29, #-16]!
    STP X0, X1, [X29, #-16]!

    RET

/*
 * OsLowpowerRestoreRegister:
 *   Restores CPU context from g_lowpowerSaveSRContext and returns through an
 *   exception-return style path (sets SPSR_EL1/ELR_EL1) so execution resumes at
 *   the caller of OsLowpowerSaveRegister.
 */
OsLowpowerRestoreRegister:
    CLREX

    LDR X29, =g_lowpowerSaveSRContext

    LDP X0, X1, [X29], #16
    LDP X2, X3, [X29], #16
    LDP X4, X5, [X29], #16
    LDP X6, X7, [X29], #16
    LDP X8, X9, [X29], #16
    LDP X10, X11, [X29], #16
    LDP X12, X13, [X29], #16
    LDP X14, X15, [X29], #16
    LDP X16, X17, [X29], #16
    LDP X18, X19, [X29], #16
    LDP X20, X21, [X29], #16
    LDP X22, X23, [X29], #16
    LDP X24, X25, [X29], #16
    LDP X26, X27, [X29], #16

    STP X2, X3, [SP, #-16]!
    LDR X3, =g_lowpowerSaveAR
    STR X0, [X3]
    STR X1, [X3, #8]
    STR X2, [X3, #16]
    LDP X2, X3, [SP], #16

    MOV X0, X29

    LDP X28, X29, [X0], #16

    /* POP {DAIF, NZCV} */
    LDP X1, X2, [X0], #16
    MRS X1, DAIF
    MRS X2, NZCV
    ORR X1, X1, X2
    ORR X1, X1, #1
    ORR X1, X1, #RUNLVL
    MSR SPSR_ELx, X1

    /* POP {SP, LR} */
    LDP X1, X30, [X0], #16
    MOV SP, X1
    MSR ELR_ELx, X30

    LDR X0, =g_lowpowerSaveAR
    LDR X0, [X0]
    LDR X1, =g_lowpowerSaveAR
    LDR X1, [X1, #8]
    LDR X2, =g_lowpowerSaveAR
    LDR X2, [X2, #16]

    RET

    .end
