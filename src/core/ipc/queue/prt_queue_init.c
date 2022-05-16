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
 * Description: 队列初始化函数实现
 */
#include "securec.h"
#include "prt_queue_external.h"
#include "prt_mem_external.h"
#include "prt_lib_external.h"

/* 队列最大个数 */
OS_SEC_BSS U16 g_maxQueue;
OS_SEC_BSS struct TagQueCb *g_allQueue;

/*
 * 描述：队列注册
 */
OS_SEC_L4_TEXT U32 OsQueueRegister(U16 maxQueue)
{
    if (maxQueue == 0) {
        return OS_ERRNO_QUEUE_MAXNUM_ZERO;
    }

    g_maxQueue = maxQueue;
    return OS_OK;
}

OS_SEC_L4_TEXT U32 OsQueueConfigInit(void)
{
    void *addr = NULL;

    addr = OsMemAlloc(OS_MID_QUEUE, OS_MEM_DEFAULT_FSC_PT, g_maxQueue * sizeof(struct TagQueCb));
    if (addr == NULL) {
        return OS_ERRNO_QUEUE_NO_MEMORY;
    }

    if (memset_s(addr, g_maxQueue * sizeof(struct TagQueCb), 0, g_maxQueue * sizeof(struct TagQueCb)) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    g_allQueue = (struct TagQueCb *)addr;

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsQueueGetNodeSize(U16 nodeSize)
{
    return ALIGN(nodeSize, OS_QUEUE_NODE_SIZE_ALIGN) + OS_QUEUE_NODE_HEAD_LEN;
}

OS_SEC_L4_TEXT U32 OsQueueCreatParaCheck(U16 nodeNum, U16 nodeSize, const U32 *queueId)
{
    if (queueId == NULL) {
        return OS_ERRNO_QUEUE_CREAT_PTR_NULL;
    }

    if ((nodeNum == 0) || (nodeSize == 0)) {
        return OS_ERRNO_QUEUE_PARA_ZERO;
    }

    if (OsQueueGetNodeSize(nodeSize) > OS_MAX_U16) {
        return OS_ERRNO_QUEUE_NSIZE_INVALID;
    }
    return OS_OK;
}

OS_SEC_L4_TEXT U32 OsQueueCreate(U16 nodeNum, U16 maxNodeSize, U32 *queueId)
{
    U32 index;
    U32 qId = 0;
    struct QueNode *queueNode = NULL;
    U16 nodeSize = maxNodeSize;
    struct TagQueCb *queueCb = NULL;

    /* 获取一个空闲的队列资源 */
    queueCb = g_allQueue;
    for (index = 0; index < g_maxQueue; index++, queueCb++) {
        if (queueCb->queueState == OS_QUEUE_UNUSED) {
            qId = OS_QUEUE_ID(index);
            break;
        }
    }

    if (index == g_maxQueue) {
        return OS_ERRNO_QUEUE_CB_UNAVAILABLE;
    }

    nodeSize = (U16)OsQueueGetNodeSize(nodeSize);
    queueCb->queue = (U8 *)OsMemAlloc(OS_MID_QUEUE, OS_MEM_DEFAULT_FSC_PT, (U32)nodeNum * (U32)nodeSize);
    if (queueCb->queue == NULL) {
        return OS_ERRNO_QUEUE_CREATE_NO_MEMORY;
    }

    for (index = 0; index < nodeNum; index++) {
        queueNode = (struct QueNode *)(uintptr_t)&queueCb->queue[index * nodeSize];
        queueNode->srcPid = OS_QUEUE_PID_INVALID;
    }

    queueCb->nodeNum = nodeNum;
    queueCb->nodeSize = nodeSize;
    queueCb->queueState = OS_QUEUE_USED;
    INIT_LIST_OBJECT(&queueCb->writeList);
    INIT_LIST_OBJECT(&queueCb->readList);
    queueCb->writableCnt = nodeNum;
    queueCb->queueHead = 0;
    queueCb->queueTail = 0;
    queueCb->nodePeak = 0;
    queueCb->readableCnt = 0;

    *queueId = qId;

    return OS_OK;
}

/*
 * 描述：创建队列接口
 */
OS_SEC_L4_TEXT U32 PRT_QueueCreate(U16 nodeNum, U16 maxNodeSize, U32 *queueId)
{
    uintptr_t intSave;
    U32 ret;
    U32 qId = 0;

    ret = OsQueueCreatParaCheck(nodeNum, maxNodeSize, queueId);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = OsIntLock();
    ret = OsQueueCreate(nodeNum, maxNodeSize, &qId);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    *queueId = qId;
    OsIntRestore(intSave);
    return OS_OK;
}
