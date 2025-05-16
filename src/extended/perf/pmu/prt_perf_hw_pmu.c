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

OS_SEC_BSS static Pmu *g_perfHw;

static char *g_eventName[PERF_COUNT_HW_MAX] = {
    [PERF_COUNT_HW_CPU_CYCLES]              = "cycles",
    [PERF_COUNT_HW_INSTRUCTIONS]            = "instructions",
    [PERF_COUNT_HW_ICACHE_REFERENCES]       = "icache",
    [PERF_COUNT_HW_ICACHE_MISSES]           = "icache-misses",
    [PERF_COUNT_HW_DCACHE_REFERENCES]       = "dcache",
    [PERF_COUNT_HW_DCACHE_MISSES]           = "dcache-misses",
    [PERF_COUNT_HW_BRANCH_INSTRUCTIONS]     = "branches",
    [PERF_COUNT_HW_BRANCH_MISSES]           = "branches-misses",
};

/**
 * 1.If config event is PERF_EVENT_TYPE_HW, then map it to the real eventId first
 * 2.Find available counter for each event.
 * 3.Decide whether this hardware pmu need prescaler (once every 64 cycle counts).
 */
static U32 OsPerfHwConfig()
{
    U32 i;
    U32 eventId;
    U32 eventNum;
    Event *event = NULL;
    PerfEvent *events = NULL;
    HwPmu *armPmu = GET_HW_PMU(g_perfHw);

    U32 maxCounter = OsGetPmuMaxCounter();
    U32 counter = OsGetPmuCounter0();
    U32 cycleCode = armPmu->mapEvent(PERF_COUNT_HW_CPU_CYCLES, PERF_EVENT_TO_CODE);
    if (cycleCode == PERF_HW_INVAILD_EVENT_TYPE) {
        printf("find cpu cycle map failed\n");
        return OS_ERROR;
    }

    events = &g_perfHw->events;
    eventNum = events->nr;
    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        if (!VALID_PERIOD(event->period)) {
            printf("Config period: 0x%x invalid, should be in (%#x, %#x)\n", event->period,
                PERIOD_CALC(CCNT_PERIOD_UPPER_BOUND), PERIOD_CALC(CCNT_PERIOD_LOWER_BOUND));
            return OS_ERROR;
        }

        if (g_perfHw->type == PERF_EVENT_TYPE_HW) {
            eventId = armPmu->mapEvent(event->eventId, PERF_EVENT_TO_CODE);
            if (eventId == PERF_HW_INVAILD_EVENT_TYPE) {
                printf("find hardware map failed, eventId = %u\n", eventId);
                return OS_ERROR;
            }
            event->eventId = eventId;
        }

        if (event->eventId == cycleCode) {
            event->counter = OsGetPmuCycleCounter();
        } else {
            event->counter = counter;
            counter++;
        }

        if (counter > maxCounter) {
            printf("max events: %u excluding cycle event\n", maxCounter - 1);
            return OS_ERROR;
        }

#ifndef LOSCFG_SHELL_PERF
        printf("Perf Config %u eventId = 0x%x, counter = 0x%x, period = 0x%x\n", i, event->eventId, event->counter, event->period);
#endif
    }

    armPmu->cntDivided = events->cntDivided & armPmu->canDivided;

    return OS_OK;
}

static U32 OsPerfHwStart()
{
    U32 i;
    Event *event = NULL;
    U32 cpuid = PRT_GetCoreID();
    HwPmu *armPmu = GET_HW_PMU(g_perfHw);
    PerfEvent *events = &g_perfHw->events;
    U32 eventNum = events->nr;

    armPmu->clear();

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        armPmu->setPeriod(event);
        armPmu->enable(event);
        event->count[cpuid] = 0;
    }

    armPmu->start();
    return OS_OK;
}

static U32 OsPerfHwStop()
{
    U32 i;
    U32 eventId;
    uintptr_t value;
    Event *event = NULL;
    U32 cpuid = PRT_GetCoreID();
    HwPmu *armPmu = GET_HW_PMU(g_perfHw);
    PerfEvent *events = &g_perfHw->events;
    U32 eventNum = events->nr;

    armPmu->stop();

    for (i = 0; i < eventNum; i++) {
        event = &(events->per[i]);
        value = armPmu->readCnt(event);
        event->count[cpuid] += value;

        eventId = armPmu->mapEvent(event->eventId, PERF_CODE_TO_EVENT);
        if ((eventId == PERF_COUNT_HW_CPU_CYCLES) && (armPmu->cntDivided != 0)) {
            /* CCNT counts every 64th cpu cycle */
            event->count[cpuid] = event->count[cpuid] << 6;
        }

#ifndef LOSCFG_SHELL_PERF
        printf("perf stop [%s] : event[0x%x] = %llu\n", g_eventName[eventId], event->eventId, event->count[cpuid]);
#endif
    }

    return OS_OK;
}

static char *OsPerfGetEventName(Event *event)
{
    U32 eventId;
    HwPmu *armPmu = GET_HW_PMU(g_perfHw);

    eventId = armPmu->mapEvent(event->eventId, PERF_CODE_TO_EVENT);
    if (eventId < PERF_COUNT_HW_MAX) {
        return g_eventName[eventId];
    } else {
        return "unknown";
    }
}

U32 OsPerfHwInit(HwPmu *hwPmu)
{
    U32 ret;

    if (hwPmu == NULL) {
        return OS_ERROR;
    }

    hwPmu->pmu.type    = PERF_EVENT_TYPE_HW;
    hwPmu->pmu.config  = OsPerfHwConfig;
    hwPmu->pmu.start   = OsPerfHwStart;
    hwPmu->pmu.stop    = OsPerfHwStop;
    hwPmu->pmu.getName = OsPerfGetEventName;

    (void)memset_s(&hwPmu->pmu.events, sizeof(hwPmu->pmu.events), 0, sizeof(hwPmu->pmu.events));
    ret = OsPerfPmuRegister(&hwPmu->pmu);

    g_perfHw = OsPerfPmuGet(PERF_EVENT_TYPE_HW);
    if (g_perfHw == NULL) {
        printf("perf get hardware pmu failed\n");
        return OS_ERRNO_PERF_INVALID_PMU;
    }

    return ret;
}