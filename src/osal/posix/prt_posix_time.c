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
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"
#include "prt_swtmr_external.h"

#define OS_SYS_NS_PER_MS  (OS_SYS_NS_PER_SECOND / OS_SYS_MS_PER_SECOND)

static struct timespec g_accDeltaFromSet;

/* 封装超时处理函数 */
static void OsTimerWrapper(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    (void)tmrHandle;
    (void)arg3;
    (void)arg4;
    void *(*sigev_notify_function)(union sigval) = (void *)arg1;

    union sigval val = {
        .sival_int = arg2
    };

    sigev_notify_function(val);
}

int timer_create(clockid_t clockId, struct sigevent * restrict evp, timer_t * restrict timerId)
{
    U32 ret;
    TimerHandle swtmrId;
    struct TimerCreatePara timer = {0};

    if ((timerId == NULL) || (evp == NULL) || (clockId != CLOCK_REALTIME)) {
        return EINVAL;
    }

    if (evp->sigev_notify != SIGEV_THREAD) { // 必须有超时处理函数
        return EINVAL;
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
        return EINVAL;
    }

    *timerId = (timer_t)swtmrId;
    return OS_OK;
}

int timer_delete(timer_t timerId)
{
    U32 swtmrId = (U32)timerId;

    if (PRT_TimerDelete(0, swtmrId) != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

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

void OsTimeMs2Spec(U32 expireTime, struct timespec *tp)
{
    U32 remainder;

    tp->tv_sec = expireTime / OS_SYS_MS_PER_SECOND;
    remainder = expireTime - (U32)tp->tv_sec * OS_SYS_MS_PER_SECOND;
    tp->tv_nsec = (long)remainder * OS_SYS_NS_PER_MS;
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

    timeOutNs = (time->tv_sec - curTime.tv_sec) * OS_SYS_NS_PER_SECOND + (time->tv_nsec - curTime.tv_nsec);
    if (timeOutNs <= 0) {
        return ETIMEDOUT;
    }

    timeOut = (U64)(time->tv_sec - curTime.tv_sec) * OsSysGetTickPerSecond();
    if (time->tv_nsec > curTime.tv_nsec) {
        timeOut += ((U64)(time->tv_nsec - curTime.tv_nsec) * OsSysGetTickPerSecond() - 1) / OS_SYS_NS_PER_SECOND + 1;
    } else {
        timeOut -= ((U64)(curTime.tv_nsec - time->tv_nsec) * OsSysGetTickPerSecond()) / OS_SYS_NS_PER_SECOND;
    }
    *ticks = timeOut >= OS_WAIT_FOREVER ? OS_WAIT_FOREVER : (U32)timeOut;

    return OS_OK;
}

int timer_gettime(timer_t timerId, struct itimerspec *value)
{
    U32 ret;
    U32 expireTime; // 剩余超时时间,单位ms
    struct SwTmrInfo info;

    if (value == NULL) {
        return EINVAL;
    }

    ret = PRT_TimerQuery(0, (TimerHandle)timerId, &expireTime);
    if (ret != OS_OK) {
        return EINVAL;
    }

    ret = PRT_SwTmrInfoGet((TimerHandle)timerId, &info);
    if (ret != OS_OK) {
        return EINVAL;
    }

    OsTimeMs2Spec(expireTime, &value->it_value);
    OsTimeMs2Spec((info.mode == OS_TIMER_ONCE) ? 0 : info.interval, &value->it_interval);

    return OS_OK;
}

int timer_settime(timer_t timerId, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    U32 intSave;
    U32 interval, expiry, ret;
    struct TagSwTmrCtrl *swtmr;
    (void)flags;

    if (value == NULL) {
        return EINVAL;
    }

    if (!OsTimeCheckSpec(&value->it_value) || !OsTimeCheckSpec(&value->it_interval)) {
        return EINVAL;
    }

    expiry = OsTimeSpec2Tick(&value->it_value);
    interval = OsTimeSpec2Tick(&value->it_interval);

    if (interval != 0 && interval != expiry) {
        return ENOTSUP;
    }

    if (ovalue) {
        if (timer_gettime(timerId, ovalue) != OS_OK) {
            return EINVAL;
        }
    }

    ret = PRT_TimerStop(0, timerId);
    if (ret != OS_OK && ret != OS_ERRNO_SWTMR_UNSTART) {
        return EINVAL;
    }

    // 当 it_value = 0, 表示要停止定时器.
    if ((value->it_value.tv_sec == 0) && (value->it_value.tv_nsec == 0)) {
        return OS_OK;
    }

    intSave = PRT_HwiLock();
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(timerId);
    swtmr->mode = (interval ? OS_TIMER_LOOP : OS_TIMER_ONCE);
    swtmr->interval = (interval ? interval : expiry);
    swtmr->idxRollNum = swtmr->interval;
    PRT_HwiRestore(intSave);

    if (PRT_TimerStart(0, timerId) != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    U64 nanosec;
    U64 tick;
    const U32 nsPerTick = OS_SYS_NS_PER_SECOND / g_tickModInfo.tickPerSecond;

    if (!OsTimeCheckSpec(rqtp)) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    nanosec = (U64)rqtp->tv_sec * OS_SYS_NS_PER_SECOND + (U64)rqtp->tv_nsec;
    tick = (nanosec + nsPerTick - 1) / nsPerTick; // 睡眠时间不得小于rqtp规定的时间

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

U32 sleep(U32 seconds)
{
    U32 ret;
    U32 ticks;
    
    ticks = seconds * g_tickModInfo.tickPerSecond;
    ret = PRT_TaskDelay(ticks ? (ticks + 1) : 0);
    if (ret != OS_OK) {
        return EINTR;
    }

    return OS_OK;
}

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

int clock_gettime(clockid_t clockId, struct timespec *tp)
{
    if (tp == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    switch (clockId) {
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_MONOTONIC_COARSE:
            OsTimeGetHwTime(tp);
            return OS_OK;
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_COARSE:
            OsTimeGetRealTime(tp);
            return OS_OK;
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
