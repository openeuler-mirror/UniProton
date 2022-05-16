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
 * Description: 队列维测函数实现
 */
#include "prt_queue_external.h"

OS_SEC_L2_TEXT U32 OsQueueGetParaCheck(U32 innerId, const U32 *num, struct TagQueCb **queueCb)
{
    if (innerId >= g_maxQueue) {
        return OS_ERRNO_QUEUE_INVALID;
    }

    if (num == NULL) {
        return OS_ERRNO_QUEUE_PTR_NULL;
    }

    *queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    if ((*queueCb)->queueState == OS_QUEUE_UNUSED) {
        return OS_ERRNO_QUEUE_NOT_CREATE;
    }
    return OS_OK;
}

/*
 * 描述：获取消息队列的历史最大使用长度。
 */
OS_SEC_L2_TEXT U32 PRT_QueueGetUsedPeak(U32 queueId, U32 *queueUsedNum)
{
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = NULL;
    uintptr_t intSave;
    U32 ret;

    intSave = OsIntLock();
    ret = OsQueueGetParaCheck(innerId, queueUsedNum, &queueCb);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    *queueUsedNum = (U32)queueCb->nodePeak;
    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * 描述：获取当前等待消息队列中指定源任务的待处理消息个数。
 */
OS_SEC_L2_TEXT U32 PRT_QueueGetNodeNum(U32 queueId, U32 taskPid, U32 *queueNum)
{
    U32 ret;
    U32 loop;
    uintptr_t intSave;
    U32 num = 0;
    U32 numAll = 0;
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = NULL;
    struct QueNode *queueNode = NULL;

    intSave = OsIntLock();
    ret = OsQueueGetParaCheck(innerId, queueNum, &queueCb);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    if (taskPid != OS_QUEUE_PID_INVALID) {
        for (loop = 0; loop < queueCb->nodeNum; loop++) {
            queueNode = (struct QueNode *)(uintptr_t)&queueCb->queue[loop * (queueCb->nodeSize)];
            if (queueNode->srcPid == taskPid) {
                num++;
            }

            if (queueNode->srcPid != OS_QUEUE_PID_INVALID) {
                numAll++;
            }
        }
    }

    *queueNum = (taskPid == OS_QUEUE_PID_ALL) ? numAll : num;

    OsIntRestore(intSave);

    return OS_OK;
}
