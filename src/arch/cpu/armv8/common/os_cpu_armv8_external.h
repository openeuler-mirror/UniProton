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
 * Description: 属性宏相关内部头文件
 */
#ifndef OS_CPU_ARMV8_EXTERNAL_H
#define OS_CPU_ARMV8_EXTERNAL_H

#include "prt_buildef.h"
#include "prt_hwi.h"
#include "prt_gic_external.h"

/*
 * 模块间宏定义
 */
#define OS_IRQ2HWI(irqNum)           (irqNum)
#define OS_HWI2IRQ(hwiNum)           (hwiNum)
#define OS_HWI_GET_HWINUM(archNum)   (archNum)
#define OS_HWI_GET_HWI_PRIO(hwiPrio) (hwiPrio)
#define OS_HWI_IS_SGI(hwiNum)        ((hwiNum) <= MAX_SGI_ID)
#define OS_HWI_IS_PPI(hwiNum)        (((hwiNum) > MAX_SGI_ID) && ((hwiNum) <= MAX_PPI_ID))

/* OS_HWI_MAX_NUM 大小会影响bss段大小。需要根据实际使用hwi个数配置 */
#define OS_HWI_MAX_NUM           0x182U
#define OS_HWI_NUM_CHECK(hwiNum) ((hwiNum) >= OS_HWI_MAX_NUM)

#define OS_HWI_MAX           (OS_HWI_MAX_NUM - 1)
#define OS_HWI_FORMARRAY_NUM OS_HWI_MAX_NUM
#define OS_HWI_MIN           0
#define OS_HWI_PRI_NUM       14

/* 中断优先级0~15，但非安全世界的中断优先级只能是偶数 */
#define OS_HWI_PRIO_CHECK(hwiPrio)     ((hwiPrio) >= OS_HWI_PRI_NUM || ((hwiPrio) & 1U))
#define OS_HWI_SET_HOOK_ATTR(hwiNum, hwiPrio, hook)

#define OS_HWI_CLEAR_CHECK(hwiNum)    ((hwiNum) == GIC_INT_ID_MASK)

#define OS_HWI_INTERNAL_NUM 5

#define OS_TICK_COUNT_UPDATE()

#define OS_HW_TICK_INIT() OS_OK

#define OS_IS_TICK_PERIOD_INVALID(cyclePerTick) (FALSE)

#define OS_TSK_STACK_SIZE_ALIGN  16U
#define OS_TSK_STACK_SIZE_ALLOC_ALIGN MEM_ADDR_ALIGN_016
#define OS_TSK_STACK_ADDR_ALIGN  16U

#define OS_MAX_CACHE_LINE_SIZE   4 /* 单核芯片定义为4 */

/* 任务栈最小值 */
#define OS_TSK_MIN_STACK_SIZE (ALIGN((0x1D0 + 0x10 + 0x4), 16))

/* Idle任务的消息队列数 */
#define OS_IDLE_TASK_QUE_NUM 1

#define DIV64(a, b) ((a) / (b))
#define DIV64_REMAIN(a, b) ((a) % (b))

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock()   PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

/* 硬件平台保存的任务上下文 */
struct TagHwContext {
    uintptr_t pc;
    uintptr_t spsr;
    uintptr_t far;
    uintptr_t esr;
    uintptr_t xzr;
    uintptr_t lr;
    uintptr_t x[30];
};

/*
 * 模块间变量声明
 */
extern uintptr_t __os_sys_sp_end;
extern uintptr_t __os_sys_sp_start;
extern uintptr_t __bss_end__;
extern uintptr_t __bss_start__;

/*
 * 模块间函数声明
 */
extern uintptr_t OsGetSysStackSP(void);
extern void OsSetSysStackSP(uintptr_t stackPointer, U32 hwiNum);
extern uintptr_t OsGetSysStackStart(void);
extern uintptr_t OsGetSysStackEnd(void);
extern void OsTaskTrap(void);
extern void OsTskContextLoad(uintptr_t stackPointer);

/*
 * 描述: 使能IRQ中断
 */
OS_SEC_ALW_INLINE INLINE void OsIntEnable(void)
{
    OS_EMBED_ASM(
        "msr    daifclr, %0"
        :
        : "i"(DAIF_IRQ_BIT) // IRQ mask
        : "memory");
}

/*
 * 描述: 使能FIQ中断
 */
OS_SEC_ALW_INLINE INLINE void OsFiqEnable(void)
{
    OS_EMBED_ASM(
        "msr    daifclr, %0"
        :
        : "i"(DAIF_FIQ_BIT) // FIQ mask
        : "memory");
}

/*
 * 描述: 禁止IRQ中断
 */
OS_SEC_ALW_INLINE INLINE void OsIntDisable(void)
{
    OS_EMBED_ASM(
        "msr    daifset, %0"
        :
        : "i"(DAIF_IRQ_BIT) // IRQ mask
        : "memory", "cc");
}
#if (OS_GIC_VER == 2)
OS_SEC_ALW_INLINE INLINE U32 OsHwiNumGet(void)
{
    U32 iar;

    iar = GIC_REG_READ(GICC_IAR);

    return (iar & IAR_MASK);
}
#else
/*
 * 描述: 获取当前PENDING的中断号, 中断状态PENDING->ACTIVE
 */
OS_SEC_ALW_INLINE INLINE U32 OsHwiNumGet(void)
{
    U32 iar;

    OS_EMBED_ASM("MRS    %0," REG_ALIAS(ICC_IAR1_EL1)" \n"
                 : "=&r"(iar) : : "memory");

    return iar;
}
#endif

#if (OS_GIC_VER == 2)
OS_SEC_ALW_INLINE INLINE void OsHwiClear(U32 intId)
{
    GIC_REG_WRITE(GICC_EOIR, intId & IAR_MASK);
}
#else
/*
 * 描述: 清除中断ACTIVE状态
 */
OS_SEC_ALW_INLINE INLINE void OsHwiClear(U32 intId)
{
    OS_EMBED_ASM("MSR " REG_ALIAS(ICC_EOIR1_EL1)", %0 \n"
                 : : "r"(intId) : "memory");
    return;
}
#endif

/*
 * 描述: 获取SP
 */
OS_SEC_ALW_INLINE INLINE uintptr_t OsGetSp(void)
{
    uintptr_t sp;

    OS_EMBED_ASM("MOV  %0, SP" : "=r"(sp));

    return sp;
}

/*
 * 描述: 传入任务切换时的栈地址
 */
OS_SEC_ALW_INLINE INLINE uintptr_t OsTskGetInstrAddr(uintptr_t addr)
{
    return ((struct TagHwContext *)addr)->pc;
}

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFast(void)
{
    OsTaskTrap();
}

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFastPs(uintptr_t intSave)
{
    (void)intSave;
    OsTaskTrap();
}

#endif /* OS_CPU_ARMV8_EXTERNAL_H */
