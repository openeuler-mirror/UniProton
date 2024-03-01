/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-13
 * Description: 属性宏相关内部头文件
 */
#ifndef OS_CPU_RISCV64_EXTERNAL_H
#define OS_CPU_RISCV64_EXTERNAL_H

#include "prt_typedef.h"
#include "prt_hwi.h"

#define OS_HWI_MAX_NUM 1024
#define OS_HWI_MIN 0
#define OS_HWI_MAX (OS_HWI_MAX_NUM - 1)

#define OS_HWI_PRI_NUM 8
#define OS_HWI_PRIO_CHECK(hwiPrio) ((hwiPrio) >= OS_HWI_PRI_NUM || (hwiPrio) == 0)
#define OS_IRQ2HWI(irqNum) ((irqNum) + OS_HWI_MIN)
#define OS_HWI2IRQ(hwiNum) ((hwiNum) - OS_HWI_MIN)
#define OS_HWI_FORMARRAY_NUM OS_HWI_MAX_NUM
#define OS_HWI_SET_HOOK_ATTR(hwiNum, hwiPrio, hook)
#define OS_HWI_GET_HWINUM(archNum) (archNum)
#define OS_HWI_NUM_CHECK(hwiNum) ((hwiNum) > OS_HWI_MAX || (hwiNum == 0))

#define OS_TICK_COUNT_UPDATE()
#define OS_HW_TICK_INIT() OS_OK
#define OS_IS_TICK_PERIOD_INVALID(cyclePerTick) (FALSE)

#define OS_TSK_STACK_SIZE_ALIGN  16U
#define OS_TSK_STACK_SIZE_ALLOC_ALIGN MEM_ADDR_ALIGN_016
#define OS_TSK_STACK_ADDR_ALIGN  16U

#define OS_IDLE_TASK_QUE_NUM 1

#define OS_MAX_CACHE_LINE_SIZE 4

/* 任务栈最小值 */
#define OS_TSK_MIN_STACK_SIZE (ALIGN((0x1D0 + 0x10 + 0x4), 16))

#define DIV64(a, b) ((a) / (b))
#define DIV64_REMAIN(a, b) ((a) % (b))

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock()   PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

#define OsIntEnable()     PRT_HwiUnLock()
#define OsIntDisable()    PRT_HwiLock()


/*
 * 模块间变量声明
 */
extern uintptr_t __os_sys_sp_end;
extern uintptr_t __os_sys_sp_start;
extern uintptr_t __bss_end__;
extern uintptr_t __bss_start__;


extern void OsTaskTrap(void);
extern void OsTskContextLoad(uintptr_t stackPointer);

#define OsTaskTrapFast()            OsTaskTrap()
#define OsTaskTrapFastPs(intSave)   OsTaskTrap()
/* 传入任务切换时的栈地址 */
#define OsTskGetInstrAddr(addr)     (((struct TskContext *)(addr))->mepc)

/*
 * 描述: 获取SP
 */
OS_SEC_ALW_INLINE INLINE uintptr_t OsGetSp(void)
{
    uintptr_t sp;
    OS_EMBED_ASM("add %0, sp, zero":"=r"(sp)::);
    return sp;
}

#endif