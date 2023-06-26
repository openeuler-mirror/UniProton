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
 * Description: timer_gettime 相关接口实现
 */
#include <time.h>
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"
#include "prt_swtmr_external.h"

#define OS_SYS_NS_PER_MS  (OS_SYS_NS_PER_SECOND / OS_SYS_MS_PER_SECOND)

void OsTimeMs2Spec(U32 expireTime, struct timespec *tp)
{
    U32 remainder;

    tp->tv_sec = expireTime / OS_SYS_MS_PER_SECOND;
    remainder = expireTime - (U32)tp->tv_sec * OS_SYS_MS_PER_SECOND;
    tp->tv_nsec = (long)remainder * OS_SYS_NS_PER_MS;
}

int timer_gettime(timer_t timerId, struct itimerspec *value)
{
    U32 ret;
    U32 expireTime; // 剩余超时时间,单位ms
    struct SwTmrInfo info;

    if (value == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    ret = PRT_TimerQuery(0, (TimerHandle)timerId, &expireTime);
    if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    ret = PRT_SwTmrInfoGet((TimerHandle)timerId, &info);
    if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    OsTimeMs2Spec(expireTime, &value->it_value);
    OsTimeMs2Spec((info.mode == OS_TIMER_ONCE) ? 0 : info.interval, &value->it_interval);

    return OS_OK;
}
