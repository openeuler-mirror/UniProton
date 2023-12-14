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
 * Description: nanosleep 相关接口实现
 */
#include <time.h>
#include "prt_posix_internal.h"
#include "prt_sys_external.h"

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    U64 nanosec;
    U64 tick;
    const U32 nsPerTick = OS_SYS_NS_PER_SECOND / g_tickModInfo.tickPerSecond;

    struct TagTskCb *curTskCb = RUNNING_TASK;
    uintptr_t intSave = OsIntLock();
    if (curTskCb->cancelState == PTHREAD_CANCEL_ENABLE && curTskCb->cancelPending) {
        PRT_PthreadExit(PTHREAD_CANCELED);
    }
    OsIntRestore(intSave);

    if (!OsTimeCheckSpec(rqtp)) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    nanosec = (U64)rqtp->tv_sec * OS_SYS_NS_PER_SECOND + (U64)rqtp->tv_nsec;
    tick = ((nanosec + nsPerTick - 1) / nsPerTick) + 1; // 睡眠时间不得小于rqtp规定的时间

    if (tick >= U32_MAX) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    if (PRT_TaskDelay((U32)tick) == OS_OK) {
        if (rmtp != NULL) {
            rmtp->tv_sec = rmtp->tv_nsec = 0;
        }
        return OS_OK;
    }

    return PTHREAD_OP_FAIL;
}