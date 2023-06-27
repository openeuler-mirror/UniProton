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
 * Description: 定时器模块
 */
#include "prt_attr_external.h"
#include "prt_timer_external.h"
#include "prt_err_external.h"

OS_SEC_BSS struct TagFuncsLibTimer g_timerApi[TIMER_TYPE_INVALID];

/*
 * 描述：创建硬件或者软件定时器
 */
OS_SEC_L4_TEXT U32 PRT_TimerCreate(struct TimerCreatePara *createPara, TimerHandle *tmrHandle)
{
    U32 ret;
    U32 timerType = TIMER_TYPE_SWTMR;

    if ((createPara == NULL) || (tmrHandle == NULL)) {
        return OS_ERRNO_TIMER_INPUT_PTR_NULL;
    }

    if (createPara->interval == 0) {
        return OS_ERRNO_TIMER_INTERVAL_INVALID;
    }

    if ((createPara->mode != OS_TIMER_LOOP) && (createPara->mode != OS_TIMER_ONCE)) {
        return OS_ERRNO_TIMER_MODE_INVALID;
    }

    switch (createPara->type) {
        case OS_TIMER_HARDWARE:
            timerType = TIMER_TYPE_HWTMR;
            break;

        case OS_TIMER_SOFTWARE:
            timerType = TIMER_TYPE_SWTMR;
            break;

        default:
            ret = OS_ERRNO_TIMER_TYPE_INVALID;
            goto OS_TIMER_CREATE_ERR;
    }

    if (g_timerApi[timerType].createTimer == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_CREATE_ERR;
    }

    ret = g_timerApi[timerType].createTimer(createPara, tmrHandle);
    return ret;

OS_TIMER_CREATE_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer create error, timer type is 0x%x\n", createPara->type);
    return ret;
}

/*
 * 描述：定时器删除函数
 */
OS_SEC_L4_TEXT U32 PRT_TimerDelete(U32 mid, TimerHandle tmrHandle)
{
    U32 ret;
    U32 timerType;
    (void)mid;

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_DEL_ERR;
    }

    if (g_timerApi[timerType].deleteTimer == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_DEL_ERR;
    }

    ret = g_timerApi[timerType].deleteTimer(tmrHandle);
    return ret;

OS_TIMER_DEL_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer del error handle 0x%x\n", tmrHandle);
    return ret;
}

/*
 * 描述：重新启动定时器
 */
OS_SEC_L2_TEXT U32 PRT_TimerRestart(U32 mid, TimerHandle tmrHandle)
{
    U32 ret;
    U32 timerType;
    (void)mid;

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_RESTART_ERR;
    }

    if (g_timerApi[timerType].restartTimer == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_RESTART_ERR;
    }

    ret = g_timerApi[timerType].restartTimer(tmrHandle);
    return ret;

OS_TIMER_RESTART_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer restart error handle 0x%x\n", tmrHandle);
    return ret;
}

/*
 * 描述：获取定时器超时次数
 */
OS_SEC_L2_TEXT U32 PRT_TimerGetOverrun(U32 mid, TimerHandle tmrHandle, U32 *overrun)
{
    U32 ret;
    U32 timerType;
    (void)mid;

    timerType = OS_TIMER_GET_TYPE(tmrHandle);
    if (timerType >= TIMER_TYPE_INVALID) {
        ret = OS_ERRNO_TIMER_HANDLE_INVALID;
        goto OS_TIMER_RESTART_ERR;
    }

    if (g_timerApi[timerType].getOverrun == NULL) {
        ret = OS_ERRNO_TIMER_NOT_INIT_OR_GROUP_NOT_CREATED;
        goto OS_TIMER_RESTART_ERR;
    }

    ret = g_timerApi[timerType].getOverrun(tmrHandle, overrun);
    return ret;

OS_TIMER_RESTART_ERR:
    OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, "timer get overrun error handle 0x%x\n", tmrHandle);
    return ret;
}