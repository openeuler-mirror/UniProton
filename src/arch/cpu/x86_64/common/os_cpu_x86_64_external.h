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
 * Create: 2023-06-25
 * Description: 属性宏相关内部头文件
 */
#ifndef OS_CPU_X86_64_EXTERNAL_H
#define OS_CPU_X86_64_EXTERNAL_H

#include "prt_typedef.h"
#include "prt_hwi.h"
#include "./hw/x86_64/os_cpu_x86_64.h"

#define OS_HWI_MAX_NUM 256
#define OS_HWI_MIN 0
#define OS_HWI_MAX (OS_HWI_MAX_NUM - 1)

#define OS_HWI_INTERNAL_NUM 5
#define OS_HWI_PRI_NUM 256
#define OS_HWI_PRIO_CHECK(hwiPrio) ((hwiPrio) >= OS_HWI_PRI_NUM)
#define OS_IRQ2HWI(irqNum) ((irqNum) + OS_HWI_MIN)
#define OS_HWI2IRQ(hwiNum) ((hwiNum) - OS_HWI_MIN)
#define OS_HWI_FORMARRAY_NUM OS_HWI_MAX_NUM
#define OS_HWI_SET_HOOK_ATTR(hwiNum, hwiPrio, hook)
#define OS_HWI_GET_HWINUM(archNum) (archNum)
#define OS_HWI_NUM_CHECK(hwiNum) ((hwiNum) > OS_HWI_MAX)

#define OS_TICK_COUNT_UPDATE()
#define OS_HW_TICK_INIT() OS_OK
#define OS_IS_TICK_PERIOD_INVALID(cyclePerTick) (FALSE)

#define NUMBER_FOR_MAGIC_WORD_ADD 0
#define OS_IDLE_TASK_QUE_NUM 1

#define OS_TSK_STACK_SIZE_ALIGN 16
#define OS_TSK_STACK_SIZE_ALLOC_ALIGN MEM_ADDR_ALIGN_016
#define OS_TSK_STACK_ADDR_ALIGN 16

#define OS_MAX_CACHE_LINE_SIZE 4

/* 任务栈最小值 */
#define OS_TSK_MIN_STACK_SIZE (ALIGN((0x1D0 + 0x10 + 0x4), 16))

#define OS_SPINLOCK_INIT_FOREACH(maxNum, structName, field)
#define OS_SPIN_FREE_FOREACH(maxNum, structName, field)
#define OS_SPIN_FREE(lockVar)

#define OsSplLock(spinLock)
#define OsSplUnlock(spinLock)
#define OsSplLockInit(spinLock) ((void)(spinLock))

#define OsIntUnLock() PRT_HwiUnLock()
#define OsIntLock()   PRT_HwiLock()
#define OsIntRestore(intSave) PRT_HwiRestore(intSave)

extern void OsTskContextLoad(uintptr_t tcbAddr);

extern U32 __bss_start__;
extern U32 __bss_end__;
extern U32 __os_sys_sp_end;
extern U32 __os_sys_sp_start;

OS_SEC_ALW_INLINE INLINE U32 OsGetLMB1(U32 value)
{
    OS_EMBED_ASM("bsr %1,%0"
         :"=r"(value)
         :"m"(value));
    return 31 - value;
}

OS_SEC_ALW_INLINE INLINE U64 osGetU64LMB1(U64 value)
{
    U64 count;
    OS_EMBED_ASM(
            "bsrq %1, %0\n\t" // bsr和mov后面的q是指8字节数据宽度，每行汇编代码结尾都要加换行符\n\t
            "jnz 1f\n\t"      // 寄存器ZF标志为0，%0中结果有效直接跳转到标号1，f是指向前跳转
            "movq $-1, %0\n\t"// 寄存器ZF标志为1，代表所有的位都是0，所以返回-1
            "1:"
            :"=q"(count):"q"(value));
    return 63 - count;
}

/* 计算一个32bit非0数字的最右位     */
/* e.g. 0x01000020 ----> 结果返回 5 */
OS_SEC_ALW_INLINE INLINE U32 OsGetRMB(U32 bit)
{
    U32 rev = bit - 1;
    rev ^= bit;
    OS_EMBED_ASM("bsr %1,%0"
         :"=r"(rev)
         :"m"(rev));
    return rev;
}

OS_SEC_ALW_INLINE INLINE U32 OsVa2Pa(const VirtAddr vAddr, PhyAddr *addr)
{
    *addr = (PhyAddr)vAddr;
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsMemAddrToOriginal(uintptr_t addr, uintptr_t referAddr)
{
    (void)referAddr;
    return addr;
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsMemAddrToUncache(uintptr_t addr)
{
    return addr;
}

OS_SEC_ALW_INLINE INLINE void OsWaitForReboot(void)
{
    return;
}

extern void OsTaskTrap();

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFast(void)
{
    OsTaskTrap();
    return;
}

OS_SEC_ALW_INLINE INLINE void OsTaskTrapFastPs(uintptr_t intSave)
{
    (void)intSave;
    OsTaskTrap();
    return;
}

OS_SEC_ALW_INLINE INLINE void OsSpinLockInitInner(volatile uintptr_t *lockVar)
{
    (void)lockVar;
    return;
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsGetSp(void)
{
    uintptr_t sp;

    OS_EMBED_ASM("mov %%rsp, %0" : "=q"(sp));

    return sp;
}

OS_SEC_ALW_INLINE INLINE uintptr_t OsTskGetInstrAddr(uintptr_t addr)
{
    return ((struct TagOsStack *)(addr + OS_FPU_SIZE))->rip;
}

#endif
