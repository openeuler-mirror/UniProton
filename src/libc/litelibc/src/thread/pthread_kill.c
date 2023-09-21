/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread_kill接口实现
 */
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "prt_signal.h"

int pthread_kill(pthread_t t, int sig)
{
    TskHandle taskId = (TskHandle)t;
    signalInfo info = {0};
    info.si_signo = sig;
    info.si_code = SI_USER;
    U32 ret = PRT_SignalDeliver(taskId, &info);
    if (ret != OS_OK) {
        return -1;
    }

    return 0;
}