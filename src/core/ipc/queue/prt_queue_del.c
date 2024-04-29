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
 * Description: 队列函数实现
 */
#include "prt_mem.h"
#include "prt_queue_external.h"

/*
 * 描述：删除队列，只提给供实验室使用
 */
OS_SEC_L4_TEXT U32 PRT_QueueDelete(U32 queueId)
{
    uintptr_t intSave;
    U32 ret;
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = NULL;

    if (innerId >= g_maxQueue) {
        return OS_ERRNO_QUEUE_INVALID;
    }

    /* 获取指定队列控制块 */
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    QUEUE_CB_IRQ_LOCK(queueCb, intSave);
    if (queueCb->queueState == OS_QUEUE_UNUSED) {
        ret = OS_ERRNO_QUEUE_NOT_CREATE;
        goto QUEUE_END;
    }

    if (!ListEmpty(&queueCb->writeList) || !ListEmpty(&queueCb->readList)) {
        ret = OS_ERRNO_QUEUE_IN_TSKUSE;
        goto QUEUE_END;
    }

    if ((queueCb->writableCnt + queueCb->readableCnt) != queueCb->nodeNum) {
        ret = OS_ERRNO_QUEUE_BUSY;
        goto QUEUE_END;
    }

    ret = PRT_MemFree((U32)OS_MID_QUEUE, (void *)(queueCb->queue));
    if (ret != OS_OK) {
        goto QUEUE_END;
    }

    queueCb->queueState = OS_QUEUE_UNUSED;

QUEUE_END:
    QUEUE_CB_IRQ_UNLOCK(queueCb, intSave);
    return ret;
}
