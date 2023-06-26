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
 * Description: clock_getres 相关接口实现
 */
#include <time.h>
#include "prt_posix_internal.h"
#include "prt_sys_external.h"

int clock_getres(clockid_t clockId, struct timespec *tp)
{
    if (tp == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    switch (clockId) {
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_REALTIME_COARSE:
            tp->tv_nsec = (long)((OS_SYS_NS_PER_SECOND - 1) / g_systemClock + 1); // 每个cycle多少ns
            tp->tv_sec = 0;
            return OS_OK;
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
            errno = ENOTSUP;
            return PTHREAD_OP_FAIL;
        default:
            errno = EINVAL;
            return PTHREAD_OP_FAIL;
    }
}

