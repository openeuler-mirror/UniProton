/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-3-16
 * Description: queue命令行实现
 */

#include "shcmd.h"
#include "prt_queue_external.h"
#include "prt_task_external.h"

static void ShowAllQueueInfo()
{
    if (g_maxQueue == 0) {
        PRINTK("queue is not enable.\n");
        return;
    }

    PRINTK("QID  state  nodeNum  nodeSize  queueHead  queueTail  nodePeak  writableCnt  readableCnt\n");
    PRINTK("---------------------------------------------------------------------------------------\n");

    U32 queueId;
    struct TagQueCb *queueCb = g_allQueue;
    for (int index = 0; index < g_maxQueue; index++, queueCb++) {
        queueId = OS_QUEUE_ID(index);
        PRINTK("%-4u %-6u %-8u %-9u %-10u %-10u %-9u %-12u %u\n",
            queueId, queueCb->queueState, queueCb->nodeNum, queueCb->nodeSize, queueCb->queueHead,
            queueCb->queueTail, queueCb->nodePeak, queueCb->writableCnt, queueCb->readableCnt);
    }

    return;
}

static void ShowQueueListInfo(const struct TagListObject *list)
{
    bool isEmpty = ListEmpty(list);
    if (isEmpty) {
        PRINTK("NULL\n");
        return;
    }
    struct TagTskCb *curTskCb = NULL;
    LIST_FOR_EACH(curTskCb, list, struct TagTskCb, pendList) {
        PRINTK("0x%x ", curTskCb->taskPid);
    }
    PRINTK("\n");
}

static void ShowQueueInfo(U32 queueId)
{
    if (g_maxQueue == 0) {
        PRINTK("queue is not enable.\n");
        return;
    }

    U32 pendingNum;
    struct TagTskCb *taskCb = NULL;

    PRINTK("taskId    PendingMsgNum\n");
    PRINTK("-----------------------\n");
    for (int i = 0; i < OS_MAX_TCB_NUM; i++) {
        taskCb = GET_TCB_HANDLE(i + g_tskBaseId);
        if (TSK_IS_UNUSED(taskCb)) {
            continue;
        }

        /* 输出任务已写入队列但未被处理的消息个数 */
        pendingNum = 0;
        PRT_QueueGetNodeNum(queueId, taskCb->taskPid, &pendingNum);
        PRINTK("%-6u    %u\n", taskCb->taskPid, pendingNum);
    }

    /* 输出队列的读写超时任务列表 */
    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    struct TagQueCb *queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    PRINTK("writeList:\n");
    ShowQueueListInfo(&queueCb->writeList);
    PRINTK("readList:\n");
    ShowQueueListInfo(&queueCb->readList);

    return;
}

UINT32 OsShellCmdQueue(UINT32 argc, const CHAR **argv)
{
    if (argc == 0) {
        ShowAllQueueInfo();
        return OS_OK;
    }

    if (!strcmp("--help", argv[0]) || argc > 1) {
        PRINTK("\nUsage: queue [ID]\n");
        return OS_OK;
    }

    char *endptr = NULL;
    U32 queueId = strtoul(argv[0], &endptr, 0);
    if (endptr == NULL || endptr == argv[0] || *endptr != '\0') {
        PRINTK("Invalid queue id.\n");
        return OS_ERROR;
    }

    U32 innerId = OS_QUEUE_INNER_ID(queueId);
    if (innerId >= g_maxQueue) {
        PRINTK("queue id out of range.\n");
        return OS_ERROR;
    }

    struct TagQueCb *queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    if (queueCb->queueState == OS_QUEUE_UNUSED) {
        PRINTK("queue is unused.\n");
        return OS_ERROR;
    }

    ShowQueueInfo(queueId);

    return OS_OK;
}

SHELLCMD_ENTRY(queue_shellcmd, CMD_TYPE_EX, "queue", 0, (CmdCallBackFunc)OsShellCmdQueue);