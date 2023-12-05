/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-12-04
 * Description: taskInfo命令行实现
 */

#include "shcmd.h"
#include "prt_task_external.h"

static void OsShellCmdListAllTask()
{
    struct TagTskCb *taskCb = NULL;
    PRINTK("Pid              Name             Status\n");
    PRINTK("---------------- ---------------- ----------------\n");
    for (int index = 0; index < OS_MAX_TCB_NUM; index++) {
        taskCb = GET_TCB_HANDLE(index + g_tskBaseId);
        if (TSK_IS_UNUSED(taskCb)) {
            continue;
        }

        PRINTK("%-16u %-16s %-16u\n", index + g_tskBaseId, taskCb->name, taskCb->taskStatus);
    }
}

int OsShellCmdTaskInfo(int argc, const char **argv)
{
    if (argc == 0) {
        OsShellCmdListAllTask();
        return 0;
    }

    if (!strcmp("help", argv[0])) {
        PRINTK("Usage: taskInfo [TASKID]");
        return 0;
    }
    char *endptr = NULL;
    U32 taskId = strtoul(argv[0], &endptr, 0);
    if (endptr == NULL || endptr == argv[0] || *endptr != '\0') {
        PRINTK("Invalid task id.");
        return 0;
    }

    if (taskId > U32_MAX) {
        PRINTK("Task id out of range.");
        return 0;
    }

    struct TskInfo taskInfo = {0};
    U32 ret = PRT_TaskGetInfo(taskId, &taskInfo);
    if (ret != 0) {
        PRINTK("Task not found.");
        return 0;
    }

    PRINTK("TaskBasicInfo:\n");
    PRINTK(" name:       %16s taskEntry:  %16p\n", taskInfo.name, taskInfo.entry);
    PRINTK(" taskSatus:  %16u taskPrio:   %16u\n", taskInfo.taskStatus, taskInfo.taskPrio);
    PRINTK(" sp:         %16p pc:         %16p\n", taskInfo.sp, taskInfo.pc);

    PRINTK("StackInfo:\n");
    PRINTK(" stackSize:  %16u topOfStack: %16p\n", taskInfo.stackSize, taskInfo.topOfStack);
    PRINTK(" stackBottom:%16p currUsed:   %16u\n", taskInfo.bottom, taskInfo.currUsed);
    PRINTK(" peakUsed:   %16u overflow:   %16s\n", taskInfo.peakUsed, (taskInfo.ovf ? "True" : "False"));
    return 0;
}

SHELLCMD_ENTRY(taskInfo_shellcmd, CMD_TYPE_EX, "taskInfo", 1, (CmdCallBackFunc)OsShellCmdTaskInfo);