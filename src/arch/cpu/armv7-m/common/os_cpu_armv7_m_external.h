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
 * Create: 2009-12-22
 * Description: mem模块的模块内头文件
 */
#ifndef OS_CPU_ARMV7_M_EXTERNAL_H
#define OS_CPU_ARMV7_M_EXTERNAL_H

#include "prt_buildef.h"
#include "prt_hwi.h"
#include "prt_clk.h"

#define OS_MAX_CACHE_LINE_SIZE 4 /* 单核芯片定义为4 */
#define OS_MX_SYS_VECTOR_CNT 16 /* 系统向量个数 16 */
#define OS_HWI_PRI_INVALID_BYTE 4 /* 芯片优先级无效位数 */

/* M4支持中断嵌套，需要设置PRIGROUP = 0， 表达抢占优先级的位段[7:1],表达子优先级的位段[0:0] */
/* 初始化中断优先级组,[1,7]都为抢占优先级位断 */
#define OS_NVIC_AIRCR_PRIGROUP 6
/* 初始化中断优先级组 */
#define OS_HWI_MAX_PRI 0x8 /* 高抢占优先级留给SVC中断 */
#define OS_HWI_PRI_HIGHEST 0x00
#define OS_HWI_PRI_LOWEST 0xF0
#define OS_MX_VECTOR_CNT (OS_MX_SYS_VECTOR_CNT + OS_MX_IRQ_VECTOR_CNT) /* 系统向量16个 + 60(M3)/82(M4)个中断向量 */
#define OS_HWI_PRI_SHIELD_HIGH 0x80 /* 屏蔽高位 */
#define OS_HWI_SET_HOOK_ATTR(hwiNum, hwiPrio, hook)

#define OS_HWI_PRIO_CHECK(hwiPrio) ((hwiPrio) >= OS_HWI_MAX_PRI)
/* 为了保持对外接口统一（硬件中断号），mx的hwinum=irqnum=真实中断号-16(内部已处理, 见OsInterrupt) */
#define OS_IRQ2HWI(irqNum) (irqNum)
#define OS_HWI2IRQ(hwiNum) (hwiNum)
#define OS_HWI_GET_HWINUM(archNum) (archNum)
#define OS_HWI_GET_HWI_PRIO(hwiPrio) (((hwiPrio) << OS_HWI_PRI_INVALID_BYTE) | OS_HWI_PRI_SHIELD_HIGH)
#define OS_HWI_GET_USER_PRIO(hwiPrio) (((hwiPrio) & (~OS_HWI_PRI_SHIELD_HIGH)) >> OS_HWI_PRI_INVALID_BYTE)

#define OS_TICK_COUNT_UPDATE()       \
    do {                             \
        (void)PRT_ClkGetCycleCount64(); \
    } while (0)

extern U32 OsTickTimerStartMx(U32 cyclePerTick);
#define OS_HW_TICK_INIT() OsTickTimerStartMx(g_cyclePerTick)
/* 检查cyclepertick是否超出寄存器范围 */
#define OS_IS_TICK_PERIOD_INVALID(cyclePerTick) ((cyclePerTick) > 0x00FFFFFF || (cyclePerTick) == 0)

/*
 * 模块间宏定义
 */
#define OS_TSK_STACK_ADDR_ALIGN 16
#define OS_TSK_STACK_SIZE_ALIGN 16
#define OS_TSK_STACK_SIZE_ALLOC_ALIGN MEM_ADDR_ALIGN_016

/* 任务栈最小值 */
#define OS_TSK_MIN_STACK_SIZE (ALIGN((0x130), 16))

/* Idle任务的消息队列数 */
#define OS_IDLE_TASK_QUE_NUM 1

extern U64 OsU64DivGetQuotient(U64 dividend, U64 divisor);
extern U64 OsU64DivGetRemainder(U64 dividend, U64 divisor);
#define DIV64(a, b) OsU64DivGetQuotient((a), (b))
#define DIV64_REMAIN(a, b) OsU64DivGetRemainder((a), (b))

/*
 * 任务上下文的结构体定义。
 */
struct TagHwContext {
    U32 r4;
    U32 r5;
    U32 r6;
    U32 r7;
    U32 r8;
    U32 r9;
    U32 r10;
    U32 r11;
    U32 basePri;
    U32 excReturn;
    U32 r0;
    U32 r1;
    U32 r2;
    U32 r3;
    U32 r12;
    U32 lr;
    U32 pc;
    U32 psr;
};

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock() PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

#define OsTaskTrap() OsTaskSwitch()
#define OsHwiTrap() OsHwiSwitch()

/*
 * 模块间内联函数定义
 */
OS_SEC_ALW_INLINE INLINE U32 OsGetHwThreadId(void)
{
    return 0x0U;
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsMemAddrToUncache(uintptr_t addr)
{
    return addr;
}

extern U32 OsGetSp(void);
extern U32 OsIntNumGet(void);
extern void OsTaskSwitch(void);
extern void OsHwiSwitch(void);
extern void OsTickIsr(void);

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFast(void)
{
    OsTaskTrap();
}

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFastPs(uintptr_t intSave)
{
    (void)intSave;
    OsTaskTrap();
}

/* 传入任务切换时的栈地址 */
OS_SEC_ALW_INLINE INLINE uintptr_t OsTskGetInstrAddr(uintptr_t addr)
{
    return ((struct TagHwContext *)addr)->pc;
}

#define OS_SPIN_LOCK_INIT(lockVar)

#if (OS_HARDWARE_PLATFORM == OS_CORTEX_M4)
#include "../cortex-m4/prt_cpu_m4_external.h"
#endif

#endif /* OS_CPU_ARMV7_M_EXTERNAL_H */
