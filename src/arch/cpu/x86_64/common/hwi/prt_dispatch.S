/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-18
 * Description: 线程调度的汇编部分。
 */
#include "prt_buildef.h"

.global OsTaskTrap
.global OsSaveRegister
.global __os_sys_sp_end
.global OsTskContextLoad, OsMainSchedule

.type OsTaskTrap, function
.type OsTskContextLoad, function
.type OsMainSchedule, function
.type OsSaveRegister, function

.section .os.kernel.text, "ax"

#define REG_SIZE      8
#define RAX_OFFSET    (0 * REG_SIZE)
#define RBX_OFFSET    (1 * REG_SIZE)
#define RCX_OFFSET    (2 * REG_SIZE)
#define RDX_OFFSET    (3 * REG_SIZE)
#define RSI_OFFSET    (4 * REG_SIZE)
#define RDI_OFFSET    (5 * REG_SIZE)
#define RBP_OFFSET    (6 * REG_SIZE)
#define R8_OFFSET     (7 * REG_SIZE)
#define R9_OFFSET     (8 * REG_SIZE)
#define R10_OFFSET    (9 * REG_SIZE)
#define R11_OFFSET    (10 * REG_SIZE)
#define R12_OFFSET    (11 * REG_SIZE)
#define R13_OFFSET    (12 * REG_SIZE)
#define R14_OFFSET    (13 * REG_SIZE)
#define R15_OFFSET    (14 * REG_SIZE)
#define INTNUM_OFFSET (15 * REG_SIZE)
#define RBPF_OFFSET   (16 * REG_SIZE)
#define RIPF_OFFSET   (17 * REG_SIZE)
#define ERROR_OFFSET  (18 * REG_SIZE)
#define RIP_OFFSET    (19 * REG_SIZE)
#define CS_OFFSET     (20 * REG_SIZE)
#define RFLAGS_OFFSET (21 * REG_SIZE)
#define RSP_OFFSET    (22 * REG_SIZE)
#define SS_OFFSET     (23 * REG_SIZE)

.macro GENERAL_REGS_SAVE
    pushq   %rax
    mov     %rsp, %rax
    and     $~15, %rax
    sub     $0x2c0, %rax
    movq    %rax, (%rdi)
#if defined(OS_OPTION_FPU)
    fxsave64  (%rax)
#endif
    add     $0x200, %rax
    popq    RAX_OFFSET(%rax)
    popq    RIP_OFFSET(%rax)
    movq    %rbx, RBX_OFFSET(%rax)
    movq    %rcx, RCX_OFFSET(%rax)
    movq    %rdx, RDX_OFFSET(%rax)
    movq    %rsi, RSI_OFFSET(%rax)
    movq    %rdi, RDI_OFFSET(%rax)
    movq    %rbp, RBP_OFFSET(%rax)
    movq    %r8,  R8_OFFSET(%rax)
    movq    %r9,  R9_OFFSET(%rax)
    movq    %r10, R10_OFFSET(%rax)
    movq    %r11, R11_OFFSET(%rax)
    movq    %r12, R12_OFFSET(%rax)
    movq    %r13, R13_OFFSET(%rax)
    movq    %r14, R14_OFFSET(%rax)
    movq    %r15, R15_OFFSET(%rax)
    movw    %cs,  CS_OFFSET(%rax)
    movw    %ss,  SS_OFFSET(%rax)
    movq    %rsp, RSP_OFFSET(%rax)
    pushfq
    popq    %rcx
    mov     %rcx, RFLAGS_OFFSET(%rax)
.endm

/*
 * 描述: Task调度处理函数 R0 is SP
 */
OsTaskTrap:
    cli
    movabs  $g_runningTask, %rdi
    movq    (%rdi), %rdi
    GENERAL_REGS_SAVE
    movabs  $__os_sys_sp_end, %rsp
    call    OsMainSchedule

OsTskContextLoad:
    movq    (%rdi), %rax
#if defined(OS_OPTION_FPU)
    fxrstor64 (%rax)
#endif
    add     $0x200, %rax
    movq    RBX_OFFSET(%rax), %rbx
    movq    RCX_OFFSET(%rax), %rcx
    movq    RDX_OFFSET(%rax), %rdx
    movq    RSI_OFFSET(%rax), %rsi
    movq    RDI_OFFSET(%rax), %rdi
    movq    RBP_OFFSET(%rax), %rbp
    movq    R8_OFFSET(%rax),  %r8
    movq    R9_OFFSET(%rax),  %r9
    movq    R10_OFFSET(%rax), %r10
    movq    R11_OFFSET(%rax), %r11
    movq    R12_OFFSET(%rax), %r12
    movq    R13_OFFSET(%rax), %r13
    movq    R14_OFFSET(%rax), %r14
    movq    R15_OFFSET(%rax), %r15
    pushq   SS_OFFSET(%rax)
    pushq   RSP_OFFSET(%rax)
    pushq   RFLAGS_OFFSET(%rax)
    pushq   CS_OFFSET(%rax)
    pushq   RIP_OFFSET(%rax)
    movq    RAX_OFFSET(%rax), %rax
    iretq
