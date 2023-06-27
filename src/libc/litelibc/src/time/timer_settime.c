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
 * Description: timer_settime 相关接口实现
 */
#include <time.h>
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"
#include "prt_swtmr_external.h"

int timer_settime(timer_t timerId, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    U32 intSave;
    U32 interval, expiry, ret;
    struct TagSwTmrCtrl *swtmr;
    (void)flags;

    if (value == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    if (!OsTimeCheckSpec(&value->it_value) || !OsTimeCheckSpec(&value->it_interval)) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    expiry = OsTimeSpec2Tick(&value->it_value);
    interval = OsTimeSpec2Tick(&value->it_interval);

    if (interval != 0 && interval != expiry) {
        errno = ENOTSUP;
        return PTHREAD_OP_FAIL;
    }

    if (ovalue) {
        if (timer_gettime(timerId, ovalue) != OS_OK) {
            errno = EINVAL;
            return PTHREAD_OP_FAIL;
        }
    }

    ret = PRT_TimerStop(0, (TimerHandle)timerId);
    if (ret != OS_OK && ret != OS_ERRNO_SWTMR_UNSTART) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    // 当 it_value = 0, 表示要停止定时器.
    if ((value->it_value.tv_sec == 0) && (value->it_value.tv_nsec == 0)) {
        return OS_OK;
    }

    intSave = PRT_HwiLock();
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX((TimerHandle)timerId);
    swtmr->mode = (interval ? OS_TIMER_LOOP : OS_TIMER_ONCE);
    swtmr->interval = (interval ? interval : expiry);
    swtmr->idxRollNum = swtmr->interval;
    swtmr->overrun = 0;
    PRT_HwiRestore(intSave);

    if (PRT_TimerStart(0, (TimerHandle)timerId) != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    return OS_OK;
}