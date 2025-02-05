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

#include "perf/prt_perf_pmu.h"

OS_SEC_BSS static SwPmu g_perfSw;

static U32 g_traceEventMap[PERF_COUNT_SW_MAX] = {
    [PERF_COUNT_SW_TASK_SWITCH]  = TASK_SWITCH,
    [PERF_COUNT_SW_HWI_RESPONSE_IN] = HWI_RESPONSE_IN,
    [PERF_COUNT_SW_MEM_ALLOC]    = MEM_ALLOC,
    [PERF_COUNT_SW_SEM_PEND]     = SEM_PEND,
};

static char* g_eventName[PERF_COUNT_SW_MAX] = {
    [PERF_COUNT_SW_TASK_SWITCH]  = "task switch",
    [PERF_COUNT_SW_HWI_RESPONSE_IN] = "irq response in",
    [PERF_COUNT_SW_MEM_ALLOC]    = "mem alloc",
    [PERF_COUNT_SW_SEM_PEND]     = "mux pend",
};

void OsPerfHook(U32 eventType)
{
    U32 i;
    U32 eventNum;
    PerfRegs regs;
    Event *event = NULL;
    PerfEvent *events = NULL;

    if (!g_perfSw.enable) {
        return;
    }

    (void)memset_s(&regs, sizeof(regs), 0, sizeof(regs));

    events = &g_perfSw.pmu.events;
    eventNum = events->nr;
    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        if (event->counter == eventType) {
            OsPerfUpdateEventCount(event, 1);
            if (event->count[PRT_GetCoreID()] % event->period == 0) {
                OsPerfFetchCallerRegs(&regs);
                OsPerfHandleOverFlow(event, &regs);
            }
            break;
        }
    }

    return;
}

static U32 OsPerfSwConfig(void)
{
    U32 i;
    Event *event = NULL;
    PerfEvent *events = &g_perfSw.pmu.events;
    U32 eventNum = events->nr;

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        if ((event->eventId < PERF_COUNT_SW_TASK_SWITCH) || (event->eventId >= PERF_COUNT_SW_MAX) || (event->period == 0)) {
            printf("software event [id:%u, period:%u] config failed\n", event->eventId, event->period);
            return OS_ERROR;
        }
        event->counter = g_traceEventMap[event->eventId];
    }

    return OS_OK;
}

static U32 OsPerfSwStart(void)
{
    U32 i;
    Event *event = NULL;
    U32 cpuid = PRT_GetCoreID();
    PerfEvent *events = &g_perfSw.pmu.events;
    U32 eventNum = events->nr;

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        event->count[cpuid] = 0;
    }

    g_perfSw.enable = TRUE;
    return OS_OK;
}

static U32 OsPerfSwStop(void)
{
    g_perfSw.enable = FALSE;
    return OS_OK;
}

static char *OsPerfGetEventName(Event *event)
{
    U32 eventId = event->eventId;
    if (eventId < PERF_COUNT_SW_MAX) {
        return g_eventName[eventId];
    }
    return "unknown";
}

U32 OsSwPmuInit(void)
{
    g_perfSw.pmu = (Pmu) {
        .type    = PERF_EVENT_TYPE_SW,
        .config  = OsPerfSwConfig,
        .start   = OsPerfSwStart,
        .stop    = OsPerfSwStop,
        .getName = OsPerfGetEventName,
    };

    g_perfSw.enable = FALSE;

    (void)memset_s(&g_perfSw.pmu.events, sizeof(g_perfSw.pmu.events), 0, sizeof(g_perfSw.pmu.events));
    return OsPerfPmuRegister(&g_perfSw.pmu);
}