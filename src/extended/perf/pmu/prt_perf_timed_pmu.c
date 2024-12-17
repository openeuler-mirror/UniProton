/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-03-16
 */

#include "prt_timer.h"
#include "perf/prt_perf_pmu.h"

OS_SEC_BSS static SwPmu g_perfTimed;
OS_SEC_BSS static U32 g_swtPmuId;

static U32 OsPerfTimedStart()
{
    U32 i;
    U32 ret;
    Event *event = NULL;
    U32 cpuid = PRT_GetCoreID();
    PerfEvent *events = &g_perfTimed.pmu.events;
    U32 eventNum = events->nr;

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        event->count[cpuid] = 0;
    }

    if (cpuid != g_primaryCoreId) {
        return OS_OK;
    }

    ret = PRT_TimerStart(0, g_swtPmuId);
    if (ret != OS_OK) {
        (void)PRT_TimerDelete(0, g_swtPmuId);
        printf("perf timer start failed, ret = 0x%x\n", ret);
        return ret;
    }

    return OS_OK;
}

static void OsPerfTimedHandle()
{
    U32 i;
    PerfRegs regs;
    Event *event = NULL;
    PerfEvent *events = &g_perfTimed.pmu.events;
    U32 eventNum = events->nr;

    (void)memset_s(&regs, sizeof(regs), 0, sizeof(regs));
    OsPerfFetchIrqRegs(&regs);

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        OsPerfUpdateEventCount(event, 1);
        OsPerfHandleOverFlow(event, &regs);
    }

    return;
}

static void OsPerfSwtimer(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    /* send to all cpu to collect data */
    SMP_CALL_PERF_FUNC(OsPerfTimedHandle);
    return;
}

static U32 OsPerfTimedConfig()
{
    U32 i;
    U32 ret;
    U32 period;
    Event *event = NULL;
    PerfEvent *events = &g_perfTimed.pmu.events;
    U32 eventNum = events->nr;

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        period = event->period;
        if (event->eventId == PERF_COUNT_CPU_CLOCK) {
            if (period == 0) {
                printf("config period invalid, period:%u ms\n", period);
                return OS_ERRNO_TIMER_INTERVAL_INVALID;
            }

            struct TimerCreatePara timer = {
                .type = OS_TIMER_SOFTWARE,
                .mode = OS_TIMER_LOOP,
                .interval = period,
                .timerGroupId = 0,
                .callBackFunc = OsPerfSwtimer,
            };

            ret = PRT_TimerCreate(&timer, &g_swtPmuId);
            if (ret != OS_OK) {
                printf("perf timer create failed, ret = 0x%x\n", ret);
                return ret;
            }

            return OS_OK;
        }
    }

    return OS_ERROR;
}

static U32 OsPerfTimedStop()
{
    U32 ret;
    if (PRT_GetCoreID() != g_primaryCoreId) {
        return OS_OK;
    }

    ret = PRT_TimerDelete(0, g_swtPmuId);
    if (ret != OS_OK) {
        printf("perf timer stop failed, ret = 0x%x\n", ret);
        return OS_ERROR;
    }

    return OS_OK;
}

static char *OsPerfGetEventName(Event *event)
{
    if (event->eventId == PERF_COUNT_CPU_CLOCK) {
        return "timed";
    } else {
        return "unknown";
    }
}

U32 OsTimedPmuInit()
{
    U32 ret;

    g_perfTimed.pmu = (Pmu) {
        .type    = PERF_EVENT_TYPE_TIMED,
        .config  = OsPerfTimedConfig,
        .start   = OsPerfTimedStart,
        .stop    = OsPerfTimedStop,
        .getName = OsPerfGetEventName,
    };

    (void)memset_s(&g_perfTimed.pmu.events, sizeof(g_perfTimed.pmu.events), 0, sizeof(g_perfTimed.pmu.events));
    ret = OsPerfPmuRegister(&g_perfTimed.pmu);
    return ret;
}