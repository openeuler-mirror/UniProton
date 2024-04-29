/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-10-05
 * Description: 核架构相关的头文件
 */
#ifndef PRT_CPU_EXTERNAL_H
#define PRT_CPU_EXTERNAL_H

#include "prt_task.h"
#include "prt_attr_external.h"

#define OS_GOTO_SYS_ERROR() \
    do {                    \
        OsAsmIll();         \
    } while (0)
#define NOP1()                                           \
    do {                                                  \
        OS_EMBED_ASM("nop" : : : "memory", "cc");         \
    } while (0)

#define NOP4()              \
    do {                    \
        NOP1();             \
        NOP1();             \
        NOP1();             \
        NOP1();             \
    } while (0)

#define NOP8()              \
    do {                    \
        NOP4();             \
        NOP4();             \
    } while (0)

#define ASM_NOP()              \
    do {                    \
        NOP8();             \
        NOP8();             \
    } while (0)

#define OS_GOTO_SYS_ERROR1() OS_GOTO_SYS_ERROR()

extern void OsAsmIll(void);
extern void OsFirstTimeSwitch(void);
extern void *OsTskContextInit(U32 taskId, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry);
extern void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context);
extern void OsTickStartRegSet(U16 tickHwTimerIndex, U32 cyclePerTick); 

#if defined(OS_ARCH_ARMV7_M)
#include "../cpu/armv7-m/common/os_cpu_armv7_m_external.h"
#endif

#if defined(OS_ARCH_ARMV8)
#include "../cpu/armv8/common/os_cpu_armv8_external.h"
#endif

#if defined(OS_ARCH_X86_64)
#include "../cpu/x86_64/common/os_cpu_x86_64_external.h"
#endif

#if defined(OS_ARCH_RISCV64)
#include "../cpu/riscv64/common/os_cpu_riscv64_external.h"
#endif

#endif /* PRT_CPU_EXTERNAL_H */
