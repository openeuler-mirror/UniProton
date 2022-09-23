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
 * Description: Hardware interrupt implementation
 */
#include "prt_hook_external.h"
#include "prt_lib_external.h"
#include "prt_task_external.h"
#include "prt_hwi_internal.h"

#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
OS_SEC_BSS U32 g_hwiMaxNumConfig;
#endif
OS_SEC_BSS OsVoidFunc g_excTrap;
/*
 * 重定位以后的向量表,
 * 其最后一个四字节表示当前运行的中断号，初始化系统定义异常向量，以便初始化阶段异常接管
 */
VEC_SEC_DATA HwiPubintFunc g_hwiTable[OS_MX_VECTOR_CNT + 1] = {
    (HwiPubintFunc)MSTACK_VECTOR,
    (HwiPubintFunc)OsResetVector,
};

OS_SEC_L4_TEXT void OsScheduleInit(void)
{
    OS_SET_VECTOR(OS_EXC_PEND_SV, OsPendSv);
    OS_SET_VECTOR(OS_EXC_SVC_CALL, OsSvchandler);

    NVIC_SET_EXC_PRI(OS_EXC_PEND_SV, OS_HWI_PRI_LOWEST);
    NVIC_SET_EXC_PRI(OS_EXC_SVC_CALL, OS_HWI_PRI_HIGHEST);
}

OS_SEC_L4_TEXT void OsHwiGICInit(void)
{
    U32 loop;
    /* 系统中断向量表初始化 */
    for (loop = 0; loop < OS_MX_SYS_VECTOR_CNT; loop++) {
        g_hwiTable[loop] = (HwiPubintFunc)OsHwiDefaultHandler;
    }
    OS_SET_VECTOR(0, (HwiPubintFunc)MSTACK_VECTOR);
    OS_SET_VECTOR(OS_EXC_RESET, OsResetVector);
    OS_SET_VECTOR(OS_EXC_NMI, OsExcNmi);
    OS_SET_VECTOR(OS_EXC_HARD_FAULT, OsExcHardFault);
    OS_SET_VECTOR(OS_EXC_MPU_FAULT, OsExcMemFault);
    OS_SET_VECTOR(OS_EXC_BUS_FAULT, OsExcBusFault);
    OS_SET_VECTOR(OS_EXC_USAGE_FAULT, OsExcUsageFault);
    g_excTrap = (OsVoidFunc)OsExcSvcCall;
    OsScheduleInit();
    OS_SET_VECTOR(OS_EXC_SYS_TICK, OsTickIsr);
    // 将Tick的优先级置为最低
    NVIC_SET_EXC_PRI(OS_EXC_SYS_TICK, OS_HWI_PRI_LOWEST);

    /* 用户中断向量表初始化 */
    for (loop = OS_MX_SYS_VECTOR_CNT; loop < OS_MX_VECTOR_CNT; loop++) {
        g_hwiTable[loop] = OsInterrupt;
    }

    /* 中断向量表定位 */
    *(volatile U32 *)OS_NVIC_VTOR = (U32)(uintptr_t)g_hwiTable;
    /*
     * 访问钥匙:任何对该寄存器的写操作都必须同时把0x05FA写入此段，否则写操作被忽略
     * 位段10:8为优先级分组
     */
    *(volatile U32 *)OS_NVIC_AIRCR = (OS_NVIC_AIRCR_REG_ACCESS_PSW |
                                      (OS_NVIC_AIRCR_PRIGROUP << OS_NVIC_AIRCR_PRIGOUP_BIT_OFFSET));
}

/*
 * 描述：默认外部中断处理函数
 */
OS_SEC_TEXT void OsInterrupt(void)
{
    HwiHandle hwiNum;
    uintptr_t intSave;

    /* 操作basepri 设置成关中断 */
    intSave = PRT_HwiLock();
    UNI_FLAG |= OS_FLG_HWI_ACTIVE;
    g_intCount++;

    /* 取外部中断号，中断号减去系统中断号 */
    hwiNum = OsIntNumGet() - OS_MX_SYS_VECTOR_CNT;
#ifdef OS_OPTION_HWI_MAX_NUM_CONFIG
    if (hwiNum > g_hwiMaxNumConfig) {
        PRT_HwiRestore(intSave);
	return;
    }
#endif
    OsTskHighestSet();

    (void)PRT_HwiUnLock();
    // ISR执行(这里一处开中断)
    OsHwiHookDispatcher(hwiNum);

    (void)PRT_HwiLock();

    g_intCount--;

    if (g_intCount == 0) {
        UNI_FLAG &= ~OS_FLG_HWI_ACTIVE;
    }

    /* 恢复 basepri */
    PRT_HwiRestore(intSave);

    if ((UNI_FLAG & OS_FLG_TSK_REQ) != 0) {
        OsHwiTrap();
    }
}

/*
 * 描述：硬中断模式设置
 */
OS_SEC_L4_TEXT void OsHwiPrioritySet(HwiHandle hwiNum, HwiPrior hwiPrio)
{
    /* 设置优先级,当前芯片高4位有效，左移4位 */
    NVIC_SET_IRQ_PRI(hwiNum, OS_HWI_GET_HWI_PRIO(hwiPrio));
}

OS_SEC_L4_TEXT U32 OsHwiPriorityGet(HwiHandle hwiNum)
{
    return (U32)OS_HWI_GET_USER_PRIO((*((volatile U8 *)((uintptr_t)OS_NVIC_PRI_BASE + (hwiNum)))));
}

/*
 * 描述：使能指定中断号
 */
OS_SEC_L2_TEXT U32 PRT_HwiEnable(HwiHandle hwiNum)
{
    uintptr_t intSave;

    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    intSave = PRT_HwiLock();

    NVIC_SET_IRQ(hwiNum);
    PRT_HwiRestore(intSave);

    return OS_OK;
}

/*
 * 描述：清除所有的中断请求位
 */
OS_SEC_L2_TEXT void PRT_HwiClearAllPending(void)
{
    uintptr_t intSave;
    U32 loop;

    intSave = PRT_HwiLock();

    for (loop = 0; loop < OS_HWI_CLRPEND_REG_NUM; loop += sizeof(U32)) {
        *(volatile U32 *)((uintptr_t)OS_NVIC_CLRPEND_BASE + loop) = OS_MAX_U32;
    }
    PRT_HwiRestore(intSave);
    return;
}

/*
 * 描述：清除单个硬中断的Pending位
 */
OS_SEC_L2_TEXT U32 PRT_HwiClearPendingBit(HwiHandle hwiNum)
{
    uintptr_t intSave;
    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    intSave = PRT_HwiLock();
    NVIC_CLR_IRQ_PEND(hwiNum);
    PRT_HwiRestore(intSave);
    return OS_OK;
}

/*
 * 描述：触发指定硬中断号
 */
OS_SEC_L2_TEXT U32 PRT_HwiTrigger(U32 dstCore, HwiHandle hwiNum)
{
    uintptr_t intSave;

    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (OsGetHwThreadId() != dstCore) {
        return OS_ERRNO_HWI_CORE_ID_INVALID;
    }

    intSave = PRT_HwiLock();

    NVIC_SET_IRQ_PEND(hwiNum);

    PRT_HwiRestore(intSave);

    return OS_OK;
}

/*
 * 描述：禁止指定中断号
 */
OS_SEC_L2_TEXT U32 PRT_HwiDisable(HwiHandle hwiNum)
{
    uintptr_t intSave;

    if (hwiNum > OS_HWI_MAX) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    intSave = PRT_HwiLock();

    NVIC_CLR_IRQ(hwiNum);
    PRT_HwiRestore(intSave);

    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_HwiDelExitHook(HwiExitHook hook)
{
    return OsHookDel(OS_HOOK_HWI_EXIT, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiAddExitHook(HwiExitHook hook)
{
    return OsHookAdd(OS_HOOK_HWI_EXIT, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiDelEntryHook(HwiEntryHook hook)
{
    return OsHookDel(OS_HOOK_HWI_ENTRY, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiAddEntryHook(HwiEntryHook hook)
{
    return OsHookAdd(OS_HOOK_HWI_ENTRY, (OsVoidFunc)hook);
}
