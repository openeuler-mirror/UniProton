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
 * Description: timer_create 相关接口实现
 */
#include "time.h"
#include "signal.h"
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"

/* 封装超时处理函数 */
static void OsTimerWrapper(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    (void)tmrHandle;
    (void)arg3;
    (void)arg4;
    void *(*sigevNotifyFunction)(union sigval) = (void *)arg1;

    union sigval val = {
        .sival_int = arg2
    };

    sigevNotifyFunction(val);
}

int timer_create(clockid_t clockId, struct sigevent * restrict evp, timer_t * restrict timerId)
{
    U32 ret;
    TimerHandle swtmrId;
    struct TimerCreatePara timer = {0};

    if ((timerId == NULL) || (evp == NULL) || (clockId != CLOCK_REALTIME)) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    if (evp->sigev_notify != SIGEV_THREAD) { // 必须有超时处理函数
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    timer.type = OS_TIMER_SOFTWARE;
    timer.mode = OS_TIMER_LOOP;
    timer.interval = 1;
    timer.timerGroupId = 0;
    timer.callBackFunc = OsTimerWrapper;
    timer.arg1 = (U32)evp->sigev_notify_function;
    timer.arg2 = (U32)evp->sigev_value.sival_int;
    ret = PRT_TimerCreate(&timer, &swtmrId);
    if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    *timerId = (timer_t)swtmrId;
    return OS_OK;
}