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
 * Create: 2022-06-18
 * Description: 硬中断。
 */
#include "prt_attr_external.h"
#include "prt_hwi.h"
#include "prt_lapic.h"
#include "prt_idt.h"
#include "../os_cpu_x86_64_external.h"

/*
 * 描述: 获取硬中断优先级
 */
OS_SEC_L4_TEXT U32 OsHwiPriorityGet(HwiHandle hwiNum)
{
    (void)hwiNum;
    return OS_OK;
}

/*
 * 描述: 设置硬中断优先级
 */
OS_SEC_L4_TEXT void OsHwiPrioritySet(HwiHandle hwiNum, HwiPrior hwiPrio)
{
    (void)hwiNum;
    (void)hwiPrio;
    return;
}

/*
 * 描述: 禁止指定中断
 */
OS_SEC_L2_TEXT U32 PRT_HwiDisable(HwiHandle hwiNum)
{
    OsIdtIrqDisable(hwiNum);
    return OS_OK;
}

/*
 * 描述: 使能指定中断
 */
OS_SEC_L2_TEXT U32 PRT_HwiEnable(HwiHandle hwiNum)
{
    OsIdtIrqEnable(hwiNum);
    return OS_OK;
}

/*
 * 描述: 清除指定中断号的pending位
 */
OS_SEC_L2_TEXT U32 PRT_HwiClearPendingBit(HwiHandle hwiNum)
{
    (void)hwiNum;
    return OS_OK;
}

/*
 * 描述: GIC模块初始化
 */
OS_SEC_L4_TEXT void OsHwiGICInit(void)
{
    OsLapicInit();
    return;
}

/*
 * 描述：使能IRQ中断
 */
OS_SEC_L0_TEXT uintptr_t PRT_HwiUnLock(void)
{
    uintptr_t intSave;
    OS_EMBED_ASM("# __raw_get_flags\n\t"
        "pushfq ; pop %0"
        : "=r" (intSave)
        : /* no input */
        : "memory");
    OS_EMBED_ASM("sti": : :"memory");
    return intSave;
}

/*
 * 描述：禁止IRQ中断
 */
OS_SEC_L0_TEXT uintptr_t PRT_HwiLock(void)
{
    uintptr_t intSave;
    OS_EMBED_ASM("# __raw_get_flags\n\t"
        "pushfq ; pop %0"
        : "=r" (intSave)
        : /* no input */
        : "memory");
    OS_EMBED_ASM("cli": : :"memory");
    return intSave;
}

/*
 * 描述: 恢复原中断状态寄存器
 */
OS_SEC_L0_TEXT void PRT_HwiRestore(uintptr_t intSave)
{
    OS_EMBED_ASM("push %0 ; popfq"
        : /* no output */
        : "g" (intSave)
        : "memory", "cc");
    return;
}