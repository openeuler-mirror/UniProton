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
 * Description: Task schedule implementation
 */
#include "prt_task_internal.h"

/*
 * 描述：AMP任务初始化
 */
OS_SEC_L4_TEXT U32 OsTskAMPInit(void)
{
    uintptr_t size;
    U32 idx;

    // 1为Idle任务
    g_threadNum += (g_tskMaxNum + 1);
    /*
     * 线程TID = LTID|PID|COREID|GTID
     */

    /* Always reserve task idle for background. */
    size = (OS_MAX_TCB_NUM) * sizeof(struct TagTskCb);

    g_tskCbArray = (struct TagTskCb *)OsMemAllocAlign((U32)OS_MID_TSK, OS_MEM_DEFAULT_FSC_PT,
                                                      size, MEM_ADDR_ALIGN_016);
    if (g_tskCbArray == NULL) {
        return OS_ERRNO_TSK_NO_MEMORY;
    }

    /* Connect all the TCBs in a doubly linked list. */
    if (memset_s(g_tskCbArray, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    g_tskBaseId = OS_SYS_PID_BASE;

    INIT_LIST_OBJECT(&g_tskCbFreeList);
    for (idx = 0; idx < OS_MAX_TCB_NUM - 1; idx++) {
        g_tskCbArray[idx].taskStatus = OS_TSK_UNUSED;
        g_tskCbArray[idx].taskPid = (idx + g_tskBaseId);
        ListTailAdd(&g_tskCbArray[idx].pendList, &g_tskCbFreeList);
    }

    /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
    RUNNING_TASK = OS_PST_ZOMBIE_TASK;

    /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
    RUNNING_TASK->taskPid = idx + g_tskBaseId;

    /* Init empty ready list for each priority. */
    for (idx = 0; idx < OS_TSK_NUM_OF_PRIORITIES; idx++) {
        INIT_LIST_OBJECT(&g_runQueue.readyList[idx]);
    }

    INIT_LIST_OBJECT(&g_tskSortedDelay.tskList);
    INIT_LIST_OBJECT(&g_tskRecyleList);

    /* 增加OS_TSK_INUSE状态，使得在Trace记录的第一条信息状态为OS_TSK_INUSE(创建状态) */
    RUNNING_TASK->taskStatus = (OS_TSK_INUSE | OS_TSK_RUNNING);
    RUNNING_TASK->priority = OS_TSK_PRIORITY_LOWEST + 1;

    return OS_OK;
}

/*
 * 描述：ilde任务创建.
 */
OS_SEC_L4_TEXT U32 OsIdleTskAMPCreate(void)
{
    U32 ret;
    TskHandle taskHdl;
    struct TskInitParam taskInitParam;
    char tskName[OS_TSK_NAME_LEN] = "IdleTask";

    /* Create background task. */
    taskInitParam.taskEntry = g_tskIdleEntry;
    taskInitParam.stackSize = g_tskModInfo.idleStackSize;
    taskInitParam.name = tskName;
    taskInitParam.taskPrio = OS_TSK_PRIORITY_LOWEST;
    taskInitParam.stackAddr = 0;

    /* 任务调度的必要条件就是有背景任务，此时背景任务还没有创建，因此不会发生任务切换 */
    ret = PRT_TaskCreate(&taskHdl, &taskInitParam);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_TaskResume(taskHdl);
    if (ret != OS_OK) {
        return ret;
    }
    IDLE_TASK_ID = taskHdl;

    return ret;
}
