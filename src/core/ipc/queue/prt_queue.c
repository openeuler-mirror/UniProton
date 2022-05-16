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
#include "prt_queue_external.h"
#include "prt_task_external.h"
#include "prt_asm_cpu_external.h"

OS_SEC_ALW_INLINE INLINE U32 OsGetSrcPid(void)
{
    U32 srcPid;

    if (OS_HWI_ACTIVE) {
        /* 硬中断创建消息不具体区别中断号 */
        srcPid = COMPOSE_PID(OsGetHwThreadId(), OS_HWI_HANDLE);
    } else {
        srcPid = RUNNING_TASK->taskPid;
    }

    return srcPid;
}

/*
 * 描述：内部Pend操作，这个函数在调用之前必须关中断。
 */
OS_SEC_ALW_INLINE INLINE U32 OsInnerPend(U16 *count, struct TagListObject *pendList, U32 timeOut)
{
    struct TagTskCb *runTsk = NULL;

    /* 判断是否需要阻塞 */
    if (*count > 0) {
        (*count)--;
        return OS_OK;
    }

    /* 阻塞任务 */
    if (timeOut == OS_QUEUE_NO_WAIT) {
        return OS_ERRNO_QUEUE_NO_SOURCE;
    }

    if (OS_INT_ACTIVE) {
        return OS_ERRNO_QUEUE_IN_INTERRUPT;
    }

    /* 如果锁任务的情况下 */
    if (OS_TASK_LOCK_DATA != 0) {
        return OS_ERRNO_QUEUE_PEND_IN_LOCK;
    }

    /* 利用局部变量 runTsk 完成对任务控制块的相关操作，不修改 RUNNING_TASK */
    runTsk = (struct TagTskCb *)RUNNING_TASK;

    /* 从任务的Ready list上把当前任务删除，添加到pend list上 */
    OsTskReadyDel(runTsk);

    TSK_STATUS_SET(runTsk, OS_TSK_QUEUE_PEND);
    ListTailAdd(&runTsk->pendList, pendList);

    /* 如果timeOut > 0,timeOut为等待时间，如果timeOut == OS_QUEUE_WAIT_FOREVER，表示永久等待 */
    if (timeOut != OS_QUEUE_WAIT_FOREVER) {
        /* 如果不是永久等待则将任务挂到计时器链表中，设置OS_TSK_TIMEOUT是为了判断是否等待超时 */
        TSK_STATUS_SET(runTsk, OS_TSK_TIMEOUT);
        OsTskTimerAdd(runTsk, timeOut);
    }

    /* 调用函数之前已经关中断，此处关中断进行调度 */
    /* 触发任务调度 */
    OsTskSchedule();
    TSK_STATUS_CLEAR(runTsk, OS_TSK_QUEUE_BUSY);

    /* 判断是否是等待队列超时 */
    if ((runTsk->taskStatus & OS_TSK_TIMEOUT) != 0) {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);

        /* 在函数的外面会开中断 */
        return OS_ERRNO_QUEUE_TIMEOUT;
    }
    /* 在函数的外面会开中断 */
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE bool OsQueuePendNeedProc(struct TagListObject *objectList)
{
    struct TagTskCb *resumedTask = NULL;

    /* 判断是否有任务阻塞于该队列 */
    if (ListEmpty(objectList)) {
        return FALSE;
    }

    /* 激活阻塞在该队列的首个任务 */
    resumedTask = GET_TCB_PEND(OS_LIST_FIRST(objectList));
    ListDelete(OS_LIST_FIRST(objectList));

    /* 去除该任务的队列阻塞位 */
    TSK_STATUS_CLEAR(resumedTask, OS_TSK_QUEUE_PEND);
    /* 如果阻塞的任务属于定时等待的任务时候，去掉其定时等待标志位，并将其从去除 */
    if ((resumedTask->taskStatus & OS_TSK_TIMEOUT) != 0) {
        /*
         * 添加PEND状态时，会加上TIMEOUT标志和timer，或者都不加。
         * 所以此时有TIMEOUT标志就一定有timer，且只有一个，且只用于该PEND方式
         */
        OS_TSK_DELAY_LOCKED_DETACH(resumedTask);
        TSK_STATUS_CLEAR(resumedTask, OS_TSK_TIMEOUT);
    }

    TSK_STATUS_SET(resumedTask, OS_TSK_QUEUE_BUSY);

    /* 如果去除队列阻塞位后，该任务不处于挂起态则将该任务挂入就绪队列并触发任务调度 */
    if ((resumedTask->taskStatus & OS_TSK_SUSPEND) == 0) {
        OsTskReadyAddBgd(resumedTask);
    }
    return TRUE;
}

/*
 * 描述：读指定队列
 */
OS_SEC_L4_TEXT U32 PRT_QueueRead(U32 queueId, void *bufferAddr, U32 *len, U32 timeOut)
{
    uintptr_t intSave;
    U32 ret;
    U32 bufLen;
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = NULL;
    struct QueNode *queueNode = NULL;

    if (innerId >= g_maxQueue) {
        return OS_ERRNO_QUEUE_INVALID;
    }

    if ((bufferAddr == NULL) || (len == NULL)) {
        return OS_ERRNO_QUEUE_PTR_NULL;
    }

    if (*len == 0) {
        return OS_ERRNO_QUEUE_SIZE_ZERO;
    }

    bufLen = *len;
    /* 获取指定队列控制块 */
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);

    intSave = OsIntLock();
    if (queueCb->queueState == OS_QUEUE_UNUSED) {
        ret = OS_ERRNO_QUEUE_NOT_CREATE;
        goto QUEUE_END;
    }

    /* 读队列PEND */
    ret = OsInnerPend(&queueCb->readableCnt, &queueCb->readList, timeOut);
    if (ret != OS_OK) {
        goto QUEUE_END;
    }

    /* 获取该队列中第一个有数据结点首地址，并将数据存入buffer中 */
    queueNode = (struct QueNode *)(uintptr_t)&queueCb->queue[(queueCb->queueHead) * (queueCb->nodeSize)];

    /* 如果buf的长度小于数据长度，则仅返回buf大小的数据，如果大于，则返回全部数据 */
    if (*len > queueNode->size) {
        *len = queueNode->size;
    }

    if (memcpy_s(bufferAddr, bufLen, (void *)queueNode->buf, *len) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    queueNode->srcPid = OS_QUEUE_PID_INVALID;

    /* 队列头指针加1 */
    queueCb->queueHead++;
    /* 如果队列头指针已经到队列尾，那么头指针指向队列头 */
    if (queueCb->queueHead == queueCb->nodeNum) {
        queueCb->queueHead = 0;
    }

    if (OsQueuePendNeedProc(&queueCb->writeList)) {
        OsTskSchedule();
        OsIntRestore(intSave);
        return OS_OK;
    }

    queueCb->writableCnt++;

QUEUE_END:
    OsIntRestore(intSave);
    return ret;
}

OS_SEC_L4_TEXT U32 OsQueueWriteParaCheck(U32 innerId, uintptr_t bufferAddr, U32 bufferSize, U32 prio)
{
    if (innerId >= g_maxQueue) {
        return OS_ERRNO_QUEUE_INVALID;
    }

    if (bufferAddr == 0) {
        return OS_ERRNO_QUEUE_PTR_NULL;
    }

    if (bufferSize == 0) {
        return OS_ERRNO_QUEUE_SIZE_ZERO;
    }

    if (prio > (U32)OS_QUEUE_URGENT) {
        return OS_ERRNO_QUEUE_PRIO_INVALID;
    }

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsQueueCpData2Node(U32 prio, uintptr_t bufferAddr, U32 bufferSize,
                                                 struct TagQueCb *queueCb)
{
    U16 peak;
    struct QueNode *queueNode = NULL;
    if (prio == (U32)OS_QUEUE_NORMAL) {
        /* 普通消息加到队列尾部 */
        queueNode = (struct QueNode *)(uintptr_t)&queueCb->queue[((queueCb->queueTail) * (queueCb->nodeSize))];

        queueCb->queueTail++;
        if (queueCb->queueTail == queueCb->nodeNum) {
            queueCb->queueTail = 0;
        }
    } else {
        /* 紧急消息加到队列头上 */
        if (queueCb->queueHead == 0) {
            queueCb->queueHead = queueCb->nodeNum;
        }
        queueCb->queueHead--;

        queueNode = (struct QueNode *)(uintptr_t)&queueCb->queue[((queueCb->queueHead) * (queueCb->nodeSize))];
    }

    if (memcpy_s((void *)queueNode->buf, (queueCb->nodeSize - OS_QUEUE_NODE_HEAD_LEN),
                 (void *)bufferAddr, bufferSize) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    queueNode->size = (U16)bufferSize;
    queueNode->srcPid = (U16)OsGetSrcPid();

    peak = queueCb->queueTail > queueCb->queueHead ? queueCb->queueTail - queueCb->queueHead
           : (queueCb->nodeNum - queueCb->queueHead) + queueCb->queueTail;

    if (peak > queueCb->nodePeak) {
        queueCb->nodePeak = peak;
    }
}

/*
 * 描述：写指定队列
 */
OS_SEC_L4_TEXT U32 PRT_QueueWrite(U32 queueId, void *bufferAddr, U32 bufferSize, U32 timeOut, U32 prio)
{
    U32 ret;
    uintptr_t intSave;
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = NULL;

    ret = OsQueueWriteParaCheck(innerId, (uintptr_t)bufferAddr, bufferSize, prio);
    if (ret != OS_OK) {
        return ret;
    }

    /* 获取指定队列控制块 */
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    intSave = OsIntLock();
    if (queueCb->queueState == OS_QUEUE_UNUSED) {
        ret = OS_ERRNO_QUEUE_NOT_CREATE;
        goto QUEUE_END;
    }

    if (bufferSize > (queueCb->nodeSize - OS_QUEUE_NODE_HEAD_LEN)) {
        ret = OS_ERRNO_QUEUE_SIZE_TOO_BIG;
        goto QUEUE_END;
    }

    /* 读队列PEND */
    ret = OsInnerPend(&queueCb->writableCnt, &queueCb->writeList, timeOut);
    if (ret != OS_OK) {
        goto QUEUE_END;
    }

    /* 选取消息节点，初始化消息节点拷贝数据 */
    OsQueueCpData2Node(prio, (uintptr_t)bufferAddr, bufferSize, queueCb);

    if (OsQueuePendNeedProc(&queueCb->readList)) {
        OsTskSchedule();
        OsIntRestore(intSave);
        return OS_OK;
    }

    queueCb->readableCnt++;

QUEUE_END:
    OsIntRestore(intSave);
    return ret;
}
