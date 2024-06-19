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
 * Create: 2023-08-30
 * Description: _Exit功能实现
 */

#include <stdlib.h>
#include "prt_task_external.h"
#include "prt_sem_external.h"

_Noreturn void _Exit(int ec)
{
    uintptr_t intSave;
    PRT_TaskLock();
    intSave = OsIntLock();
#if !defined(OS_OPTION_SMP)
    g_uniTaskLock = 1;
#endif
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_RUNNING && g_tskCbArray[i].retval != NULL) {
            *((int *)g_tskCbArray[i].retval) = ec;
            continue;
        }
        if (((OS_TSK_PEND | OS_TSK_QUEUE_PEND | OS_TSK_RW_PEND) & g_tskCbArray[i].taskStatus) != 0) {
            ListDelete(&g_tskCbArray[i].pendList);
        }

        if (((OS_TSK_DELAY | OS_TSK_TIMEOUT) & g_tskCbArray[i].taskStatus) != 0) {
            ListDelete(&g_tskCbArray[i].timerList);
        }

        if ((OS_TSK_READY & g_tskCbArray[i].taskStatus) != 0) {
            OsTskReadyDel(&g_tskCbArray[i]);
        }

        g_tskCbArray[i].taskStatus &= (~(OS_TSK_SUSPEND));
        g_tskCbArray[i].taskStatus |= OS_TSK_UNUSED;
        if (g_tskCbArray[i].retval != NULL) {
            *((int *)g_tskCbArray[i].retval) = ec;
        }
    }
    OsIntRestore(intSave);
    PRT_TaskUnlock();
    for (;;) (void)OsTaskDelete(RUNNING_TASK->taskPid);
}
