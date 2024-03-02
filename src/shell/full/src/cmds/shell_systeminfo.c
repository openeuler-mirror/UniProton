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
 * Description: systeminfo命令行实现
 */

#include "shcmd.h"
#include "prt_sem.h"
#include "prt_sem_external.h"
#include "prt_task_external.h"
#include "prt_queue_external.h"
#include "prt_swtmr_external.h"

static void ShowTaskInfo()
{
    if (g_tskMaxNum == 0) {
        return;
    }

    struct TagTskCb *taskCb = NULL;
    int cnt = 0;

    for (int index = 0; index < OS_MAX_TCB_NUM; index++) {
        taskCb = GET_TCB_HANDLE(index + g_tskBaseId);
        if (TSK_IS_UNUSED(taskCb)) {
            continue;
        }
        cnt++;
    }

    PRINTK("%-6s    %-4u    %u\n", "Task", cnt, OS_MAX_TCB_NUM);
}

static void ShowSemInfo()
{
    if (g_maxSem == 0) {
        return;
    }

    struct TagSemCb *semGet = NULL;
    int cnt = 0;

    for (int i = 0; i < g_maxSem; i++) {
        semGet = GET_SEM(i);
        if (semGet->semStat == OS_SEM_UNUSED) {
            continue;
        }
        cnt++;
    }

    PRINTK("%-6s    %-4u    %u\n", "Sem", cnt, g_maxSem);
}

static void ShowQueueInfo()
{
    if (g_maxQueue == 0) {
        return;
    }

    int cnt = 0;
    struct TagQueCb *queueCb = g_allQueue;
    for (int index = 0; index < g_maxQueue; index++, queueCb++) {
        if (queueCb->queueState == OS_QUEUE_UNUSED) {
            continue;
        }
        cnt++;
    }

    PRINTK("%-6s    %-4u    %u\n", "Queue", cnt, g_maxQueue);
    return;
}

static void ShowSwtmrInfo()
{
    if (g_swTmrMaxNum == 0) {
        return;
    }

    int cnt = 0;
    struct TagSwTmrCtrl *swtmr = NULL;
    for (int timerId = 0; timerId < g_swTmrMaxNum; timerId++) {
        swtmr = g_swtmrCbArray + timerId;
        if (swtmr->state == (U8)OS_TIMER_FREE) {
            continue;
        }
        cnt++;
    }

    PRINTK("%-6s    %-4u    %u\n", "Swtmr", cnt, g_swTmrMaxNum);
    return;
}

UINT32 OsShellCmdSysteminfo(UINT32 argc, const CHAR **argv)
{
    if (argc > 0) {
        PRINTK("\nUsage: systeminfo\n");
        return OS_ERROR;
    }

    PRINTK("Module    Used    Total\n");
    PRINTK("-----------------------\n");
    ShowTaskInfo();
    ShowSemInfo();
    ShowQueueInfo();
    ShowSwtmrInfo();

    return OS_OK;
}

SHELLCMD_ENTRY(systeminfo_shellcmd, CMD_TYPE_EX, "systeminfo", 0, (CmdCallBackFunc)OsShellCmdSysteminfo);
