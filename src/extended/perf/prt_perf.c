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
 * Create: 2024-03-21
 * Description: Perf模块
 */

#include "prt_perf.h"
#include "prt_perf_pmu.h"
#include "prt_perf_output.h"
#include "prt_atomic.h"
#include "prt_stacktrace.h"

#define MIN(x, y)             ((x) < (y) ? (x) : (y))
#define PERF_UNLOCK(state)     PRT_SplIrqUnlock(&g_perfSpin, (state))
#define PERF_LOCK(state) do { \
    (state) = PRT_SplIrqLock(&g_perfSpin); \
} while (0)

OS_SEC_BSS static Pmu *g_pmu;
OS_SEC_BSS static PerfCB g_perfCb;
OS_SEC_BSS volatile static struct PrtSpinLock g_perfSpin;

static U32 OsPmuInit(void)
{
#ifdef OS_OPTION_PERF_HW_PMU
    if (OsHwPmuInit() != OS_OK) {
        return OS_ERRNO_PERF_HW_INIT_ERROR;
    }
#endif

#ifdef OS_OPTION_PERF_TIMED_PMU
    if (OsTimedPmuInit() != OS_OK) {
        return OS_ERRNO_PERF_TIMED_INIT_ERROR;
    }
#endif

#ifdef OS_OPTION_PERF_SW_PMU
    if (OsSwPmuInit() != OS_OK) {
        return OS_ERRNO_PERF_SW_INIT_ERROR;
    }
#endif

    return OS_OK;
}

static U32 OsPerfConfig(PerfEventConfig *eventsCfg)
{
    U32 i;
    U32 ret;
    U32 eventNum;

    g_pmu = OsPerfPmuGet(eventsCfg->type);
    if (g_pmu == NULL) {
        printf("perf config type invalid %u!\n", eventsCfg->type);
        return OS_ERRNO_PERF_INVALID_PMU;
    }

    eventNum = MIN(eventsCfg->eventsNr, PERF_MAX_EVENT);
    (void)memset_s(&g_pmu->events, sizeof(PerfEvent), 0, sizeof(PerfEvent));
    for (i = 0; i < eventNum; i++) {
        g_pmu->events.per[i].eventId = eventsCfg->events[i].eventId;
        g_pmu->events.per[i].period = eventsCfg->events[i].period;
    }
    g_pmu->events.nr = i;
    g_pmu->events.cntDivided = eventsCfg->predivided;
    g_pmu->type = eventsCfg->type;

    ret = g_pmu->config();
    if (ret != OS_OK) {
        printf("perf config failed, ret = 0x%x\n", ret);
        (void)memset_s(&g_pmu->events, sizeof(PerfEvent), 0, sizeof(PerfEvent));
        return OS_ERRNO_PERF_PMU_CONFIG_ERROR;
    }

    return OS_OK;
}

static void OsPerfPrintCount(void)
{
    U32 index;
    U32 intSave;
    Event *event = NULL;
    U32 cpuid = PRT_GetCoreID();
    PerfEvent *events = &g_pmu->events;
    U32 eventNum = events->nr;

    // 主核在PRT_PerfStop时已关中断并拿到锁，这里只有从核才需要
    if (cpuid != g_primaryCoreId) {
        PERF_LOCK(intSave);
    }
    for (index = 0; index < eventNum; index++) {
        event = &(events->per[index]);

        /* filter out event counter with no event binded. */
        if (event->period == 0) {
            continue;
        }
        PRT_Printf("[%s] eventType: 0x%x [core %u]: %llu\n", g_pmu->getName(event), event->eventId, cpuid, event->count[cpuid]);
    }
    if (cpuid != g_primaryCoreId) {
        PERF_UNLOCK(intSave);
    }

    return;
}

static inline void OsPerfPrintTs(void)
{
    U64 time = OS_SYS_US_PER_SECOND * (g_perfCb.endTime - g_perfCb.startTime) / g_tickModInfo.tickPerSecond / g_systemClock;
    PRT_Printf("time used: %llu(us)\r\n", time);
}

static void OsPerfStart(void)
{
    U32 ret;
    U32 cpuid = PRT_GetCoreID();

    if (g_pmu == NULL) {
        printf("when start perf, pmu not registered!\n");
        return;
    }

    if (g_perfCb.pmuStatusPerCpu[cpuid] != PERF_PMU_STARTED) {
        ret = g_pmu->start();
        if (ret != OS_OK) {
            printf("perf start on core:%u failed, ret = 0x%x\n", cpuid, ret);
            return;
        }

        g_perfCb.pmuStatusPerCpu[cpuid] = PERF_PMU_STARTED;
    } else {
        printf("core:%u status err 0x%x\n", cpuid, g_perfCb.pmuStatusPerCpu[cpuid]);
    }

    return;
}

static void OsPerfStop(void)
{
    U32 ret;
    U32 cpuid = PRT_GetCoreID();

    if (g_pmu == NULL) {
        printf("when stop perf, pmu not registered!\n");
        return;
    }

    if (g_perfCb.pmuStatusPerCpu[cpuid] != PERF_PMU_STOPED) {
        ret = g_pmu->stop();
        if (ret != OS_OK) {
            printf("perf stop on core:%u failed, ret = 0x%x\n", cpuid, ret);
            return;
        }

        if (!g_perfCb.needStoreToBuffer) {
            OsPerfPrintCount();
        }

        g_perfCb.pmuStatusPerCpu[cpuid] = PERF_PMU_STOPED;
    } else {
        printf("core:%u status err 0x%x\n", cpuid, g_perfCb.pmuStatusPerCpu[cpuid]);
    }

    return;
}

static U32 OsPerfBackTrace(uintptr_t *callChain, U32 maxDepth, PerfRegs *regs)
{
    U32 i;
    U32 ret;

    struct TagTskCb *runningTask = RUNNING_TASK;
    ret = PRT_GetStackTraceByTaskID(&maxDepth, callChain, runningTask->taskPid);
    if (ret != OS_OK) {
        return 0;
    }

    return maxDepth;
}

static U32 OsPerfCollectData(Event *event, PerfSampleData *data, PerfRegs *regs)
{
    U32 ret;
    U32 size = 0;
    U32 depth;
    TskHandle threadId;
    U32 sampleType = g_perfCb.sampleType;
    char *p = (char *)data;

    if (sampleType & PERF_RECORD_CPU) {
        *(U32 *)(p + size) = PRT_GetCoreID();
        size += sizeof(data->cpuid);
    }

    if (sampleType & PERF_RECORD_TID) {
        struct TagTskCb *runTask = RUNNING_TASK;
        ret = PRT_TaskSelf(&threadId);
        if (ret != OS_OK) {
            PRT_Printf("when perf collect data, get task info failed, ret = 0x%x\n", ret);
            return;
        }
        *(U32 *)(p + size) = threadId;
        size += sizeof(data->taskId);
    }

    if (sampleType & PERF_RECORD_TYPE) {
        *(U32 *)(p + size) = event->eventId;
        size += sizeof(data->eventId);
    }

    if (sampleType & PERF_RECORD_PERIOD) {
        *(U32 *)(p + size) = event->period;
        size += sizeof(data->period);
    }

    if (sampleType & PERF_RECORD_TIMESTAMP) {
        *(U64 *)(p + size) = PRT_ClkGetCycleCount64();
        size += sizeof(data->time);
    }

    if (sampleType & PERF_RECORD_IP) {
        *(uintptr_t *)(p + size) = regs->pc;
        size += sizeof(data->pc);
    }

    if (sampleType & PERF_RECORD_CALLCHAIN) {
        depth = OsPerfBackTrace((uintptr_t *)(p + size + sizeof(data->callChain.ipNr)), PERF_MAX_CALLCHAIN_DEPTH, regs);
        *(U32 *)(p + size) = depth;
        size += sizeof(data->callChain.ipNr) + PERF_MAX_CALLCHAIN_DEPTH * sizeof(data->callChain.ip[0]);
    }

    return size;
}

/*
 * return TRUE if the taskId in the taskId list, return FALSE otherwise;
 * return TRUE if user haven't specified any taskId(which is supposed
 * to instrument the whole system)
 */
static int OsPerfTaskFilter(U32 taskId)
{
    U32 i;

    if (!g_perfCb.taskIdsNr) {
        return TRUE;
    }

    for (i = 0; i < g_perfCb.taskIdsNr; i++) {
        if (g_perfCb.taskIds[i] == taskId) {
            return TRUE;
        }
    }

    return FALSE;
}

static inline bool OsPerfParamValid(void)
{
    U32 index;
    U32 res = 0;
    U32 eventNum;
    PerfEvent *events = NULL;

    if (g_pmu == NULL) {
        return FALSE;
    }

    events = &g_pmu->events;
    eventNum = events->nr;
    for (index = 0; index < eventNum; index++) {
        res |= events->per[index].period;
    }

    if (res == 0) {
        return FALSE;
    }

    return TRUE;
}

static U32 OsPerfHdrInit(U32 id)
{
    PerfDataHdr head = {
        .magic      = PERF_DATA_MAGIC_WORD,
        .sampleType = g_perfCb.sampleType,
        .sectionId  = id,
        .eventType  = g_pmu->type,
        .len        = sizeof(PerfDataHdr),
    };

    return OsPerfOutPutWrite((char *)&head, head.len);
}

void OsPerfUpdateEventCount(Event *event, U32 value)
{
    if (event == NULL) {
        return;
    }

    /* event->count is U64 */
    event->count[PRT_GetCoreID()] += (value & 0xFFFFFFFF);

    return;
}

void OsPerfHandleOverFlow(Event *event, PerfRegs *regs)
{
    U32 ret;
    U32 len;
    TskHandle threadId;
    PerfSampleData data;

    ret = PRT_TaskSelf(&threadId);
    if (ret != OS_OK) {
        PRT_Printf("when perf handle over flow, get task info failed, ret = 0x%x\n", ret);
        return;
    }

    if ((g_perfCb.needStoreToBuffer) && OsPerfTaskFilter(threadId)) {
        (void)memset_s(&data, sizeof(PerfSampleData), 0, sizeof(PerfSampleData));
        len = OsPerfCollectData(event, &data, regs);
        OsPerfOutPutWrite((char *)&data, len);
    }

    return;
}

U32 PRT_PerfInit(void *buf, U32 size)
{
    U32 ret;
    U32 intSave;

#if defined(OS_OPTION_SMP)
    ret = PRT_SplLockInit(&g_perfSpin);
    if (ret != OS_OK) {
        PRT_Printf("perf spin lock init failed, ret = 0x%x\n", ret);
        return OS_ERROR;
    }
#endif

    PERF_LOCK(intSave);
    if (g_perfCb.status != PERF_UNINIT) {
        ret = OS_ERRNO_PERF_STATUS_INVALID;
        goto PERF_INIT_ERROR;
    }

    ret = OsPmuInit();
    if (ret != OS_OK) {
        goto PERF_INIT_ERROR;
    }

    ret = OsPerfOutPutInit(buf, size);
    if (ret != OS_OK) {
        ret = OS_ERRNO_PERF_BUF_ERROR;
        goto PERF_INIT_ERROR;
    }
    g_perfCb.status = PERF_STOPED;
PERF_INIT_ERROR:
    PERF_UNLOCK(intSave);

    return ret;
}

U32 PRT_PerfConfig(PerfConfigAttr *attr)
{
    U32 ret;
    U32 intSave;

    if (attr == NULL) {
        return OS_ERRNO_PERF_CONFIG_NULL;
    }

    PERF_LOCK(intSave);
    if (g_perfCb.status != PERF_STOPED) {
        ret = OS_ERRNO_PERF_STATUS_INVALID;
        PRT_Printf("perf config status error : 0x%x\n", g_perfCb.status);
        goto PERF_CONFIG_ERROR;
    }

    g_pmu = NULL;

    g_perfCb.needStoreToBuffer = attr->needStoreToBuffer;
    g_perfCb.taskFilterEnable = attr->taskFilterEnable;
    g_perfCb.sampleType = attr->sampleType;

    if (attr->taskFilterEnable) {
        ret = memcpy_s(g_perfCb.taskIds, PERF_MAX_FILTER_TSKS * sizeof(U32), attr->taskIds,
                       attr->taskIdsNr * sizeof(U32));
        if (ret != OS_OK) {
            PRT_Printf("perf copy filter task failed, ret = 0x%x\n", ret);
            goto PERF_CONFIG_ERROR;
        }
        g_perfCb.taskIdsNr = MIN(attr->taskIdsNr, PERF_MAX_FILTER_TSKS);
    }

    ret = OsPerfConfig(&attr->eventsCfg);

PERF_CONFIG_ERROR:
    PERF_UNLOCK(intSave);

    return ret;
}

void PRT_PerfStart(U32 sectionId)
{
    U32 ret;
    U32 intSave;

    PERF_LOCK(intSave);
    if (g_perfCb.status != PERF_STOPED) {
        PRT_Printf("perf start status error : 0x%x\n", g_perfCb.status);
        goto PERF_START_ERROR;
    }

    if (OsPerfParamValid() == FALSE) {
        PRT_Printf("forgot call `PRT_Config(...)` before instrumenting?\n");
        goto PERF_START_ERROR;
    }

    if (g_perfCb.needStoreToBuffer) {
        ret = OsPerfHdrInit(sectionId);
        if (ret != OS_OK) {
            PRT_Printf("perf header init error 0x%x\n", ret);
            goto PERF_START_ERROR;
        }
    }

    /* send to all cpu to start pmu */
    SMP_CALL_PERF_FUNC(OsPerfStart);
    g_perfCb.status = PERF_STARTED;
    g_perfCb.startTime = OsCurCycleGet64();
PERF_START_ERROR:
    PERF_UNLOCK(intSave);

    return;
}

void PRT_PerfStop(void)
{
    U32 intSave;

    PERF_LOCK(intSave);
    if (g_perfCb.status != PERF_STARTED) {
        PRT_Printf("perf stop status error : 0x%x\n", g_perfCb.status);
        goto PERF_STOP_ERROR;
    }

    SMP_CALL_PERF_FUNC(OsPerfStop);

    OsPerfOutPutFlush();

    g_perfCb.status = PERF_STOPED;
    g_perfCb.endTime = OsCurCycleGet64();

    OsPerfPrintTs();
PERF_STOP_ERROR:
    PERF_UNLOCK(intSave);

    return;
}

U32 PRT_PerfDataRead(char *dest, U32 size)
{
    return OsPerfOutPutRead(dest, size);
}

void PRT_PerfNotifyHookReg(const PERF_BUF_NOTIFY_HOOK func)
{
    U32 intSave;

    PERF_LOCK(intSave);
    OsPerfNotifyHookReg(func);
    PERF_UNLOCK(intSave);

    return;
}

void PRT_PerfFlushHookReg(const PERF_BUF_FLUSH_HOOK func)
{
    U32 intSave;

    PERF_LOCK(intSave);
    OsPerfFlushHookReg(func);
    PERF_UNLOCK(intSave);

    return;
}

void OsPerfSetIrqRegs(uintptr_t pc, uintptr_t fp)
{
    struct TagTskCb *runTask = RUNNING_TASK;
    runTask->pc = pc;
    runTask->fp = fp;

    return;
}
