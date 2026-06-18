/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2026-06-16
 * Description: CMSIS-RTOS adapter common functions for UniProton.
 */

#include "cmsis_uniproton.h"

bool CmsisKernelStarted(void)
{
    return OS_SYS_TASK_STATUS(UNI_FLAG);
}

bool CmsisKernelLocked(void)
{
    return OS_TASK_LOCK_DATA > 0;
}

bool CmsisPrioIsValid(int32_t priority)
{
#if (CMSIS_OS_VER == 1)
    return (priority >= -3) && (priority <= 3);
#else
    return (priority >= (int32_t)osPriorityIdle) && (priority <= (int32_t)osPriorityISR);
#endif
}

U16 CmsisPrioToPrt(int32_t priority)
{
#if (CMSIS_OS_VER == 1)
    return (U16)(32 - priority * 8);
#else
    return (U16)(OS_TSK_PRIORITY_LOWEST - priority);
#endif
}

int32_t CmsisPrioFromPrt(TskPrior priority)
{
#if (CMSIS_OS_VER == 1)
    int32_t cmsisPrio = (32 - (int32_t)priority) / 8;
    return ((cmsisPrio >= -3) && (cmsisPrio <= 3)) ? cmsisPrio : -1;
#else
    int32_t cmsisPrio = (int32_t)OS_TSK_PRIORITY_LOWEST - (int32_t)priority;
    return ((cmsisPrio >= (int32_t)osPriorityIdle) && (cmsisPrio <= (int32_t)osPriorityISR)) ? cmsisPrio : -1;
#endif
}

U32 CmsisMsToTick(U32 millisec)
{
    U64 ticks;
    U32 tickPerSecond = OsSysGetTickPerSecond();

    if (millisec == 0) {
        return 0;
    }
    if (millisec == 0xFFFFFFFFU) {
        return 0xFFFFFFFFU;
    }

    ticks = ((U64)millisec * tickPerSecond + (OS_SYS_MS_PER_SECOND - 1U)) / OS_SYS_MS_PER_SECOND;
    if (ticks == 0) {
        return 1;
    }
    return (ticks > 0xFFFFFFFFU) ? 0xFFFFFFFFU : (U32)ticks;
}

U32 CmsisTickToMs(U32 ticks)
{
    U64 ms;
    U32 tickPerSecond = OsSysGetTickPerSecond();

    if (ticks == 0) {
        return 0;
    }
    ms = ((U64)ticks * OS_SYS_MS_PER_SECOND + tickPerSecond - 1U) / tickPerSecond;
    return (ms == 0) ? 1U : ((ms > 0xFFFFFFFFU) ? 0xFFFFFFFFU : (U32)ms);
}

U32 CmsisCreateTask(TskHandle *taskId, TskEntryFunc entry, uintptr_t arg0, uintptr_t arg1,
    const char *name, U32 stackSize, U16 prio)
{
    struct TskInitParam param = {0};

    if ((taskId == NULL) || (entry == NULL)) {
        return OS_ERROR;
    }

    param.taskEntry = entry;
    param.taskPrio = prio;
    param.stackSize = stackSize;
    param.name = (char *)name;
    param.args[0] = arg0;
    param.args[1] = arg1;

    if (PRT_TaskCreate(taskId, &param) != OS_OK) {
        return OS_ERROR;
    }

    if (PRT_TaskResume(*taskId) != OS_OK) {
        (void)PRT_TaskDelete(*taskId);
        return OS_ERROR;
    }

    return OS_OK;
}
