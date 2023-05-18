/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: pthread功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

pthread_t pthread_self(void)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if (!tskCb) {
        OS_GOTO_SYS_ERROR1();
        return (pthread_t)-1;
    }

    return (pthread_t)tskCb->taskPid;
}

int pthread_equal(pthread_t pt1, pthread_t pt2)
{
    return (pt1 == pt2);
}