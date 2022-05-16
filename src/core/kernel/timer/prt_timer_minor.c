/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: Hardware timer implementation
 */
#include "prt_timer_external.h"
#include "prt_err_external.h"
#include "prt_attr_external.h"

/*
 * 描述：启动硬件定时器计时
 */
OS_SEC_L2_TEXT U32 PRT_TimerStart(U32 mid, TimerHandle tmrHandle)
{
    U32 ret;
    U32 timerType;

    (void)mid;

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_START_ERR;
    }

    if (g_timerApi[timerType].startTimer == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_START_ERR;
    }

    ret = g_timerApi[timerType].startTimer(tmrHandle);
    return ret;

OS_TIMER_START_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer start error handle 0x%x\n", tmrHandle);
    return ret;
}

/*
 * 描述：停止定时器计时
 */
OS_SEC_L2_TEXT U32 PRT_TimerStop(U32 mid, TimerHandle tmrHandle)
{
    U32 ret;
    U32 timerType;

    (void)mid;

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_STOP_ERR;
    }

    if (g_timerApi[timerType].stopTimer == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_STOP_ERR;
    }

    ret = g_timerApi[timerType].stopTimer(tmrHandle);
    return ret;

OS_TIMER_STOP_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer stop error handle 0x%x\n", tmrHandle);
    return ret;
}

/*
 * 描述：查询定时器剩余超时时间
 */
OS_SEC_L2_TEXT U32 PRT_TimerQuery(U32 mid, TimerHandle tmrHandle, U32 *expireTime)
{
    U32 ret;
    U32 timerType;
    (void)mid;

    // 检查指针是否为NULL
    if (expireTime == NULL) {
        return OS_ERRNO_TIMER_INPUT_PTR_NULL;
    }

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_QUERY_ERR;
    }

    // 检查钩子是否为NULL
    if (g_timerApi[timerType].timerQuery == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_QUERY_ERR;
    }

    ret = g_timerApi[timerType].timerQuery(tmrHandle, expireTime);
    return ret;

OS_TIMER_QUERY_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer query error handle 0x%x\n", tmrHandle);
    return ret;
}
