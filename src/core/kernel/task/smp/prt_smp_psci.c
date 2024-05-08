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
 * Create: 2024-01-26
 * Description: PSCI方式唤醒从核
 */

#include "prt_gic_external.h"
#include "prt_irq_external.h"
#include "prt_smp_task_internal.h"
#include "prt_smp_psci_internal.h"

OS_SEC_L4_TEXT U32 OsInvokePsciSmc(U64 functionId, U64 arg0, U64 arg1, U64 arg2) {
    return OsArmSmccSmc(functionId, arg0, arg1, arg2, 0, 0, 0, 0);
}

OS_SEC_L4_TEXT void OsSmpWakeUpSecondaryCore(void)
{
    U32 core, idx;
    U32 baseMpidr;
    U32 offset;
    volatile U32 cpuState;
    uintptr_t funAddr = PSCI_CPU_ON_ENTRY;

    offset = OS_MPIDR_OFFSET;
    baseMpidr = OS_MPIDR_VALID_COREID;

    for (idx = 1, core = g_cfgPrimaryCore + 1; idx < g_numOfCores; core++, idx++) {
        (void)OsInvokePsciSmc(PSCI_CPU_ON, baseMpidr + (idx << offset), funAddr, 0);
        cpuState = PSCI_CPU_STATE_OFF;
        while (cpuState != PSCI_CPU_STATE_ON)
        {
            cpuState = OsInvokePsciSmc(PSCI_AFFINITY_INFO, baseMpidr + (idx << offset), 0, 0);
        }
    }
    /* 解复位，从核开始运行 */
    g_secondaryResetFlag = 1;
    PRT_MemWait();
    for (int i = 0; i < 2; i++) {
        OsWakeUpProcessors();
    }
}

#if defined(OS_OPTION_POWEROFF)
OS_SEC_TEXT void OsCpuPowerOff(void)
{
    U32 ret;
    uintptr_t intSave;

    U32 coreId;
    U32 baseMpidr;
    U32 offset;
    volatile U32 cpuState;

    coreId = PRT_GetCoreID();

    if (coreId == g_cfgPrimaryCore) {
        OsHwiMcTrigger(OS_TYPE_TRIGGER_TO_OTHER, 0, OS_HWI_IPI_NO_02);
    }
    
    intSave = PRT_HwiLock();
    /* 去使能所有中断，防止pending中断遗留，导致下次拉核导致程序跑飞 */
    if (coreId == g_cfgPrimaryCore) {
        OsHwiDisableAll();
    }
    offset = OS_MPIDR_OFFSET;
    baseMpidr = OS_MPIDR_VALID_COREID;

    if (coreId <(U32)(g_maxNumOfCores - 1U)) {
        do {
            cpuState = OsInvokePsciSmc(PSCI_AFFINITY_INFO, baseMpidr + (1U << offset), 0, 0);
        } while (cpuState != PSCI_CPU_STATE_OFF);
    }

#ifdef OS_OPTION_OPENAMP
    if (g_setOfflineFlagHook != NULL && coreId == g_cfgPrimaryCore) {
        g_setOfflineFlagHook();
    }
#endif

    /* 刷L1 ICACHE和DCACHE */
    os_asm_flush_dcache_all();
    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
    os_asm_clean_dcache_all();

    /* 清除中断Active状态 */
    OsHwiClear(OS_HWI_IPI_NO_02); /* 基于中断的power off */
    OsHwiClear(OS_HWI_IPI_NO_07); /* 基于消息的power off */

    /* SMC陷入异常 */
    (void)OsInvokePsciSmc(PSCI_FN_CPU_OFF, 0, 0, 0);

    /* 正常offline的话不会来到这里 */
    PRT_HwiRestore(intSave);
    while (1);
}
#endif