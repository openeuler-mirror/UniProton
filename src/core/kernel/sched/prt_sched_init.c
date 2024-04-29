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
 * Create: 2024-01-25
 * Description: 运行队列初始化，构造调度框架
 */
#include "prt_sched_external.h"
#include "prt_task_external.h"
OS_SEC_BSS struct TagOsRunQue *g_runQueuePtr[OS_MAX_CORE_NUM]; // 核的局部运行队列的指针

/*
 * 描述：sched调度队列初始化
 * 备注：NA
 */
INIT_SEC_L4_TEXT void OsSchedRunQueInit(void)
{
    U32 i;
    U32 j;
    U32 prioIdx;
    struct RtRq *rtRq = NULL;
    for (i = 0; i < g_maxNumOfCores; i++) {
        g_runQueue[i].rqCoreId = i;
        g_runQueue[i].currntPrio = OS_TSK_NUM_OF_PRIORITIES;
        g_runQueuePtr[i] = &g_runQueue[i];
        rtRq = &g_runQueue[i].rtRq;
        rtRq->nextPrio = OS_TSK_NUM_OF_PRIORITIES;
        OS_LIST_INIT(&rtRq->pushAblList.nodeList);

        for(prioIdx = 0; prioIdx < OS_TSK_NUM_OF_PRIORITIES; prioIdx++) {
            OS_LIST_INIT(&rtRq->activeTsk.readyList[prioIdx]);
        }
        g_runQueue[i].schedClass = OsSchedGetSchedClass();
    }
    return;
}