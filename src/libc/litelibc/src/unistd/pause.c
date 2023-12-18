/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-12-06
 * Description: posix pause功能实现
 */

#include <unistd.h>
#include "syscall.h"
#include "prt_task_external.h"

/* 直接从就绪队列中删除 */
int pause(void)
{
    uintptr_t intSave;
    struct TagTskCb *runTask = NULL;

    intSave = OsIntLock();
    runTask = RUNNING_TASK;
    TSK_STATUS_SET(runTask, OS_TSK_SIG_PAUSE);
    OsTskReadyDel(runTask);
    OsTskSchedule();
    OsIntRestore(intSave);
    errno = EINVAL;
    return -1;
}
