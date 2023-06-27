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
 * Description: time功能实现
 */
#include "time.h"
#include "signal.h"
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"
#include "prt_swtmr_external.h"

static struct timespec g_accDeltaFromSet;

void OsTimeGetHwTime(struct timespec *hwTime)
{
    U64 cycle = OsCurCycleGet64();
    U64 nowNsec = (cycle / g_systemClock) * OS_SYS_NS_PER_SECOND +
        (cycle % g_systemClock) * OS_SYS_NS_PER_SECOND / g_systemClock;

    hwTime->tv_sec = nowNsec / OS_SYS_NS_PER_SECOND;
    hwTime->tv_nsec = nowNsec % OS_SYS_NS_PER_SECOND;
}

void OsTimeSetRealTime(const struct timespec *realTime)
{
    U32 intSave;
    struct timespec hwTime = {0};

    OsTimeGetHwTime(&hwTime);
    intSave = PRT_HwiLock();
    g_accDeltaFromSet.tv_nsec = realTime->tv_nsec - hwTime.tv_nsec;
    g_accDeltaFromSet.tv_sec = (realTime->tv_sec - hwTime.tv_sec) - (time_t)(g_accDeltaFromSet.tv_nsec < 0);
    g_accDeltaFromSet.tv_nsec = (g_accDeltaFromSet.tv_nsec + OS_SYS_NS_PER_SECOND) % OS_SYS_NS_PER_SECOND;
    PRT_HwiRestore(intSave);
}

void OsTimeGetRealTime(struct timespec *realTime)
{
    U32 intSave;
    struct timespec hwTime = {0};

    OsTimeGetHwTime(&hwTime);

    intSave = PRT_HwiLock();
    realTime->tv_nsec = hwTime.tv_nsec + g_accDeltaFromSet.tv_nsec;
    realTime->tv_sec = hwTime.tv_sec + g_accDeltaFromSet.tv_sec + (time_t)(realTime->tv_nsec >= OS_SYS_NS_PER_SECOND);
    realTime->tv_nsec %= OS_SYS_NS_PER_SECOND;
    PRT_HwiRestore(intSave);
}

bool OsTimeCheckSpec(const struct timespec *tp)
{
    if (tp == NULL) {
        return FALSE;
    }

    if ((tp->tv_sec < 0) || (tp->tv_nsec < 0) || (tp->tv_nsec >= OS_SYS_NS_PER_SECOND)) {
        return FALSE;
    }

    return TRUE;
}

void OsTimeSpec2Ms(const struct timespec *tp, U32 *expireTime)
{
    U32 tickCnt;

    tickCnt = OsTimeSpec2Tick(tp);

    *expireTime = (tickCnt / g_tickModInfo.tickPerSecond) * OS_SYS_MS_PER_SECOND;
}

U32 OsTimeSpec2Tick(const struct timespec *tp)
{
    U64 ns;
    U64 tmp;
    U64 tickCnt;
    U32 nsPerTick;

    nsPerTick = OS_SYS_NS_PER_SECOND / OsSysGetTickPerSecond(); // 每个tick多少纳秒
    ns = (U64)tp->tv_sec * OS_SYS_NS_PER_SECOND + (U64)tp->tv_nsec;
    tickCnt = ns / nsPerTick;
    tmp = ns - tickCnt * nsPerTick;
    if (tmp > 0) {
        tickCnt++;
    }

    if (tickCnt > U32_MAX) {
        tickCnt = U32_MAX;
    }

    return (U32)tickCnt;
}

U32 OsTimeOut2Ticks(const struct timespec *time, U32 *ticks)
{
    struct timespec curTime;
    S64 timeOutNs;
    U64 timeOut;

    if ((time->tv_nsec < 0) || (time->tv_nsec >= OS_SYS_NS_PER_SECOND) ||
        (time->tv_sec < 0) || (time->tv_sec > S32_MAX)) {
        return EINVAL;
    }

    OsTimeGetRealTime(&curTime);

    timeOutNs = (S64)(time->tv_sec - curTime.tv_sec) * OS_SYS_NS_PER_SECOND + (time->tv_nsec - curTime.tv_nsec);
    if (timeOutNs <= 0) {
        return ETIMEDOUT;
    }

    timeOut = (U64)(time->tv_sec - curTime.tv_sec) * OsSysGetTickPerSecond();
    if (time->tv_nsec > curTime.tv_nsec) {
        timeOut += ((U64)(time->tv_nsec - curTime.tv_nsec) * OsSysGetTickPerSecond() - 1) / OS_SYS_NS_PER_SECOND + 1;
    } else {
        timeOut -= ((U64)(curTime.tv_nsec - time->tv_nsec) * OsSysGetTickPerSecond()) / OS_SYS_NS_PER_SECOND;
    }
    // 因为tick误差在ms级，为了确保延时时间满足要求，向上加一
    timeOut = timeOut + 1;
    *ticks = timeOut >= OS_WAIT_FOREVER ? OS_WAIT_FOREVER : (U32)timeOut;

    return OS_OK;
}