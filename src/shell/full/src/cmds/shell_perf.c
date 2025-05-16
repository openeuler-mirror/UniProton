#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "prt_perf.h"
#include "prt_perf_pmu.h"
#include "prt_task.h"
#include "prt_sys_external.h"

#define PERF_INTERVAL_US 1000
#define PERF_BUF_LEN 0x10000

static bool g_perfInited = false;
OS_SEC_BSS static char g_perfBuffer[PERF_BUF_LEN];
extern void OsPrintBuff(const char *buf, U32 len);

static void PerfHwEventStat(U32 tick)
{
    U32 ret;
    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_HW,
            .events = {
                [0] = {PERF_COUNT_HW_CPU_CYCLES, 0xFFFF},
                [1] = {PERF_COUNT_HW_INSTRUCTIONS, 0xFFFFFF00},
                [2] = {PERF_COUNT_HW_DCACHE_REFERENCES, 0xFFFFFF00},
                [3] = {PERF_COUNT_HW_DCACHE_MISSES, 0xFFFFFF00},
                [4] = {PERF_COUNT_HW_ICACHE_REFERENCES, 0xFFFFFF00},
                [5] = {PERF_COUNT_HW_ICACHE_MISSES, 0xFFFFFF00},
                [6] = {PERF_COUNT_HW_BRANCH_INSTRUCTIONS, 0xFFFFFF00},
            },
            .eventsNr = 7,
            .predivided = 1,
        },
        .taskIds = {0},
        .taskIdsNr = 0,
        .needStoreToBuffer = FALSE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        printf("perf config hw event error 0x%x\n", ret);
        return;
    }

    PRT_PerfStart(1);
    PRT_TaskDelay(tick);
    PRT_PerfStop();
}

static void PerfSwEventStat(U32 tick)
{
    U32 ret;
    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_SW,
            .events = {
                [0] = {PERF_COUNT_SW_TASK_SWITCH, 1},
                [1] = {PERF_COUNT_SW_HWI_RESPONSE_IN, 10000},
                [2] = {PERF_COUNT_SW_MEM_ALLOC, 1},
                [3] = {PERF_COUNT_SW_SEM_PEND, 1},
            },
            .eventsNr = 4,
            .predivided = 0,
        },
        .taskIds = {0},
        .taskIdsNr = 0,
        .needStoreToBuffer = FALSE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        printf("perf config sw event error 0x%x\n", ret);
        return;
    }

    PRT_PerfStart(1);
    PRT_TaskDelay(tick);
    PRT_PerfStop();
}

static void PerfTimedEventStat(U32 tick)
{
    U32 ret;
    U32 len;
    char readBuf[PERF_BUF_LEN] = {0};
    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_TIMED,
            .events = {
                [0] = {PERF_COUNT_CPU_CLOCK, 1},
            },
            .eventsNr = 1,
            .predivided = 0,
        },
        .taskIds = {0},
        .taskIdsNr = 0,
        .needStoreToBuffer = TRUE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        printf("perf config timed event error 0x%x\n", ret);
        return;
    }

    PRT_PerfStart(0);
    PRT_TaskDelay(tick);
    PRT_PerfStop();
    len = PRT_PerfDataRead(readBuf, PERF_BUF_LEN);
    OsPrintBuff(readBuf, len);
}

static void PerfStatFunc(U64 interval)
{
    U32 ret;
    U32 tick;
    tick = (U32)(((double)interval / OS_SYS_US_PER_SECOND) * OsSysGetTickPerSecond());

    if (!g_perfInited) {
        memset_s(g_perfBuffer, PERF_BUF_LEN, 0, PERF_BUF_LEN);
        ret = PRT_PerfInit(g_perfBuffer, PERF_BUF_LEN);
        if (ret != OS_OK) {
            printf("Perf init failed, ret = 0x%x\n", ret);
        } else {
            g_perfInited = true;
        }
    }

#ifdef OS_OPTION_PERF_HW_PMU
    PerfHwEventStat(tick);
#elif defined(OS_OPTION_PERF_SW_PMU)
    PerfSwEventStat(tick);
#elif defined(OS_OPTION_PERF_TIMED_PMU)
    PerfTimedEventStat(tick);
#else
    printf("Perf type doesn't configured\n");
#endif
}

int OsShellCmdPerf(int argc, const char **argv)
{
    U64 interval;
    char *endptr;
    if (argc == 0) {
        PerfStatFunc(PERF_INTERVAL_US);
        return OS_OK;
    }

    if ((argc == 2) && (!strcmp("-i", argv[0]))) {
        interval = (U64)strtoul(argv[1], &endptr, 0);
        if (endptr == NULL || *endptr != '\0') {
            goto invalid_cmd_input;
        }
        PerfStatFunc(interval);
        return OS_OK;
    }

invalid_cmd_input:
    printf("\nUsage: perf [-i] [interval]\n");
    return OS_OK;
}
