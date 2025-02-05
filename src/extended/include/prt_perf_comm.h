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
 * Create: 2024-03-15
 * Description: Perf
 */

#ifndef PRT_PERF_COMM_H
#define PRT_PERF_COMM_H

#include "prt_perf.h"
#include "prt_pmu_external.h"
#include "prt_task_external.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PERF_EVENT_TO_CODE       0
#define PERF_CODE_TO_EVENT       1
#define PERF_DATA_MAGIC_WORD     0xEFEFEF00

#ifdef OS_OPTION_SMP
#define PRT_KERNEL_CORE_NUM                          OS_MAX_CORE_NUM
#else
#define PRT_KERNEL_CORE_NUM                          1
#endif

#define SMP_CALL_PERF_FUNC(func) func()

enum PmuStatus {
    PERF_PMU_STOPED,
    PERF_PMU_STARTED,
};

typedef struct {
    uintptr_t pc;
    uintptr_t fp;
} PerfRegs;

typedef struct {
    uintptr_t ipNr;
    uintptr_t ip[PERF_MAX_CALLCHAIN_DEPTH];
} PerfBackTrace;

typedef struct {
    U32           cpuid;                /* cpu id */
    U32           taskId;               /* task id */
    U32           eventId;              /* record type */
    U32           period;               /* record period */
    U64           time;                 /* record time */
    uintptr_t     pc;                   /* instruction pointer */
    PerfBackTrace callChain;            /* number of callChain ips */
} PerfSampleData;

typedef struct {
    U32             magic;           /* magic number */
    PerfEventType   eventType;       /* event type */
    U32             len;             /* sample data file length */
    PerfSampleType  sampleType;      /* IP | TID | TIMESTAMP... */
    U32             sectionId;       /* section id */
} PerfDataHdr;

typedef struct {
    U32 counter; /* 定时器类型用不到，软件事件转换为定义的对应枚举事件值，硬件事件对应真实的PMU硬件编码 */
    U32 eventId;
    U32 period;
    U64 count[PRT_KERNEL_CORE_NUM];
} Event;

typedef struct {
    Event per[PERF_MAX_EVENT];
    U8 nr;
    U8 cntDivided;
} PerfEvent;

typedef struct {
    PerfEventType type;
    PerfEvent events;
    U32 (*config)(void);
    U32 (*start)(void);
    U32 (*stop)(void);
    char *(*getName)(Event *event);
} Pmu;

typedef struct {
    /* time info */
    U64                  startTime;
    U64                  endTime;

    /* instrumentation status */
    enum PerfStatus      status;
    enum PmuStatus       pmuStatusPerCpu[PRT_KERNEL_CORE_NUM];

    /* configuration info */
    PerfSampleType       sampleType;
    U32                  taskIds[PERF_MAX_FILTER_TSKS];
    U8                   taskIdsNr;
    bool                 needStoreToBuffer;
    bool                 taskFilterEnable;
} PerfCB;

static inline void OsPerfFetchIrqRegs(PerfRegs *regs)
{
    struct TagTskCb *curTask = RUNNING_TASK;
    OsPerfArchFetchIrqRegs(regs, curTask);
}

static inline void OsPerfFetchCallerRegs(PerfRegs *regs)
{
    OsPerfArchFetchCallerRegs(regs);
}

extern void OsPerfSetIrqRegs(uintptr_t pc, uintptr_t fp);
extern void OsPerfUpdateEventCount(Event *event, U32 value);
extern void OsPerfHandleOverFlow(Event *event, PerfRegs *regs);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* PRT_PERF_COMM_H */
