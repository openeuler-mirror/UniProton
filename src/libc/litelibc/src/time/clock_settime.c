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
 * Description: clock_settime 相关接口实现
 */
#include <time.h>
#include <errno.h>
#include "prt_posix_internal.h"

int clock_settime(clockid_t clockId, const struct timespec *tp)
{
    if (!OsTimeCheckSpec(tp)) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    switch (clockId) {
        case CLOCK_REALTIME:
            OsTimeSetRealTime(tp);
            return OS_OK;
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
            errno = ENOTSUP;
            return PTHREAD_OP_FAIL;
        case CLOCK_MONOTONIC:
        default:
            errno = EINVAL;
            return PTHREAD_OP_FAIL;
    }
}
