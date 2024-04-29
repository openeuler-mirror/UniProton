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
 * Description: 实时调度私有头文件
 */
#ifndef PRT_RT_INTERNAL_H
#define PRT_RT_INTERNAL_H

#include "prt_sched_external.h"
#include "prt_task_external.h"

#if defined(OS_OPTION_SMP)
/*
 * 描述: 添加任务到就绪队列
 * 备注: NA
 */
OS_SEC_ALW_INLINE INLINE void OsEnqueueReadyListOnly(struct TagTskCb *task, bool head,
    TskPrior priority, struct RtRq *rtRq)
{
    struct TagListObject *readyList = &(rtRq->activeTsk.readyList[priority]);
    if (UNLIKELY(head)) {
        ListAdd(&task->pendList, readyList);
    } else {
        ListTailAdd(&task->pendList, readyList);
    }
    
    rtRq->activeTsk.readyPrioBit[OS_GET_32BIT_ARRAY_INDEX((U32)priority)] |= OS_32BIT_MASK(priority);

    return;
}
/*
 * 描述: 将任务从就绪队列移除
 * 备注: NA
 */
OS_SEC_ALW_INLINE INLINE void OsDequeueReadyListOnly(struct TagOsRunQue *runQue, struct TagTskCb *task)
{
    TskPrior priority = task->priority;
    struct RtRq *rtRq = &runQue->rtRq;
    struct TagListObject *readyList = &(rtRq->activeTsk.readyList[priority]);

    ListDelete(&task->pendList);

    if (ListEmpty(readyList)) {
        rtRq->activeTsk.readyPrioBit[OS_GET_32BIT_ARRAY_INDEX((U32)priority)] &= OS_32BIT_VERSE_MASK(priority);
    }
    return;
}
#endif
#endif