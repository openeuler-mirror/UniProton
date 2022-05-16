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
 * Description: Task模块
 */
#ifndef PRT_AMP_TASK_INTERNAL_H
#define PRT_AMP_TASK_INTERNAL_H

#include "prt_task_external.h"

#define OS_TSK_EN_QUE(runQue, tsk, flags) OsEnqueueTaskAmp((runQue), (tsk))
#define OS_TSK_EN_QUE_HEAD(runQue, tsk, flags) OsEnqueueTaskHeadAmp((runQue), (tsk))
#define OS_TSK_DE_QUE(runQue, tsk, flags) OsDequeueTaskAmp((runQue), (tsk))

extern U32 OsTskAMPInit(void);
extern U32 OsIdleTskAMPCreate(void);

OS_SEC_ALW_INLINE INLINE struct TagListObject *OsGetReadyList(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    TskPrior priority;
    struct TagListObject *readyList = NULL;
    U32 *tskReadyListBitMap = NULL;
    U32 *tskChildBitMap = NULL;

    priority = tsk->priority;

    readyList = &runQue->readyList[priority];
    tskReadyListBitMap = &runQue->taskReadyListBitMap;
    tskChildBitMap = &runQue->tskChildBitMap[0];

    /* if first task that is added then update */
    /* toggle the corresponing bit in g_readyPrioritiesBitMap */
    if (ListEmpty(readyList)) {
        /* 设置当前优先级所在的任务就绪链表子主BitMap表对应位 */
        *tskReadyListBitMap |= OS_SET_RDY_TSK_BIT_MAP(priority);

        /* 设置当前优先级所在的子BitMap对应位 */
        tskChildBitMap[priority >> OS_TSK_PRIO_BIT_MAP_POW] |= OS_SET_CHILD_BIT_MAP(priority);
    }

    return readyList;
}

OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    struct TagListObject *readyList = NULL;

    readyList = OsGetReadyList(runQue, tsk);
    ListTailAdd(&tsk->pendList, readyList);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskHeadAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    struct TagListObject *readyList = NULL;

    readyList = OsGetReadyList(runQue, tsk);
    ListAdd(&tsk->pendList, readyList);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsDequeueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
{
    TskPrior priority;
    struct TagListObject *readyList = NULL;
    U32 *tskReadyListBitMap = NULL;
    U32 *tskChildBitMap = NULL;

    priority = tsk->priority;
    readyList = &runQue->readyList[priority];

    ListDelete(&tsk->pendList);

    tskReadyListBitMap = &runQue->taskReadyListBitMap;
    tskChildBitMap = &runQue->tskChildBitMap[0];

    /* if first task that is added then update */
    /* toggle the corresponing bit in g_readyPrioritiesBitMap */
    if (ListEmpty(readyList)) {
        /* 清除当前优先级所在的子BitMap对应位 */
        tskChildBitMap[priority >> OS_TSK_PRIO_BIT_MAP_POW] &= OS_CLR_CHILD_BIT_MAP(priority);

        /* 当前优先级所在的任务就绪链表子BitMap全为0需要清除主BitMap表对应位 */
        if (tskChildBitMap[priority >> OS_TSK_PRIO_BIT_MAP_POW] == 0) {
            *tskReadyListBitMap &= OS_CLR_RDY_TSK_BIT_MAP(priority);
        }
    }

    return;
}

#endif /* PRT_AMP_TASK_INTERNAL_H */
