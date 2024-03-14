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