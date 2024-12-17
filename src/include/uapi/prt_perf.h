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
 * Description: Perf模块的对外头文件。
 */

#ifndef PRT_PERF_H
#define PRT_PERF_H

#include "prt_typedef.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

// Trace mask is used to filter events in runtime
typedef enum {
    TRACE_SYS_FLAG          = 0x10,
    TRACE_HWI_FLAG          = 0x20,
    TRACE_TASK_FLAG         = 0x40,
    TRACE_SWTMR_FLAG        = 0x80,
    TRACE_MEM_FLAG          = 0x100,
    TRACE_QUE_FLAG          = 0x200,
    TRACE_EVENT_FLAG        = 0x400,
    TRACE_SEM_FLAG          = 0x800,
    TRACE_MUX_FLAG          = 0x1000,

    TRACE_MAX_FLAG          = 0x80000000,
    TRACE_USER_DEFAULT_FLAG = 0xFFFFFFF0,
} PRT_TRACE_MASK;

/*
 * Trace event type which indicate the exactly happend events, user can define own module's event type like
 * TRACE_#MODULE#_FLAG | NUMBER.
 *                   28                     4
 *    0 0 0 0 0 0 0 0 X X X X X X X X 0 0 0 0 0 0
 *    |                                   |     |
 *             trace_module_flag           number
 *
 */
typedef enum {
    /* 0x10~0x1F */
    SYS_ERROR             = TRACE_SYS_FLAG | 0,
    SYS_START             = TRACE_SYS_FLAG | 1,
    SYS_STOP              = TRACE_SYS_FLAG | 2,

    /* 0x20~0x2F */
    HWI_CREATE              = TRACE_HWI_FLAG | 0,
    HWI_CREATE_SHARE        = TRACE_HWI_FLAG | 1,
    HWI_DELETE              = TRACE_HWI_FLAG | 2,
    HWI_DELETE_SHARE        = TRACE_HWI_FLAG | 3,
    HWI_RESPONSE_IN         = TRACE_HWI_FLAG | 4,
    HWI_RESPONSE_OUT        = TRACE_HWI_FLAG | 5,
    HWI_ENABLE              = TRACE_HWI_FLAG | 6,
    HWI_DISABLE             = TRACE_HWI_FLAG | 7,
    HWI_TRIGGER             = TRACE_HWI_FLAG | 8,
    HWI_SETPRI              = TRACE_HWI_FLAG | 9,
    HWI_CLEAR               = TRACE_HWI_FLAG | 10,
    HWI_SETAFFINITY         = TRACE_HWI_FLAG | 11,
    HWI_SENDIPI             = TRACE_HWI_FLAG | 12,

    /* 0x40~0x4F */
    TASK_CREATE           = TRACE_TASK_FLAG | 0,
    TASK_PRIOSET          = TRACE_TASK_FLAG | 1,
    TASK_DELETE           = TRACE_TASK_FLAG | 2,
    TASK_SUSPEND          = TRACE_TASK_FLAG | 3,
    TASK_RESUME           = TRACE_TASK_FLAG | 4,
    TASK_SWITCH           = TRACE_TASK_FLAG | 5,
    TASK_SIGNAL           = TRACE_TASK_FLAG | 6,

    /* 0x80~0x8F */
    SWTMR_CREATE          = TRACE_SWTMR_FLAG | 0,
    SWTMR_DELETE          = TRACE_SWTMR_FLAG | 1,
    SWTMR_START           = TRACE_SWTMR_FLAG | 2,
    SWTMR_STOP            = TRACE_SWTMR_FLAG | 3,
    SWTMR_EXPIRED         = TRACE_SWTMR_FLAG | 4,

    /* 0x100~0x10F */
    MEM_ALLOC             = TRACE_MEM_FLAG | 0,
    MEM_ALLOC_ALIGN       = TRACE_MEM_FLAG | 1,
    MEM_REALLOC           = TRACE_MEM_FLAG | 2,
    MEM_FREE              = TRACE_MEM_FLAG | 3,
    MEM_INFO_REQ          = TRACE_MEM_FLAG | 4,
    MEM_INFO              = TRACE_MEM_FLAG | 5,

    /* 0x200~0x20F */
    QUEUE_CREATE          = TRACE_QUE_FLAG | 0,
    QUEUE_DELETE          = TRACE_QUE_FLAG | 1,
    QUEUE_RW              = TRACE_QUE_FLAG | 2,

    /* 0x400~0x40F */
    EVENT_CREATE          = TRACE_EVENT_FLAG | 0,
    EVENT_DELETE          = TRACE_EVENT_FLAG | 1,
    EVENT_READ            = TRACE_EVENT_FLAG | 2,
    EVENT_WRITE           = TRACE_EVENT_FLAG | 3,
    EVENT_CLEAR           = TRACE_EVENT_FLAG | 4,

    /* 0x800~0x80F */
    SEM_CREATE            = TRACE_SEM_FLAG | 0,
    SEM_DELETE            = TRACE_SEM_FLAG | 1,
    SEM_PEND              = TRACE_SEM_FLAG | 2,
    SEM_POST              = TRACE_SEM_FLAG | 3,

    /* 0x1000~0x100F */
    MUX_CREATE            = TRACE_MUX_FLAG | 0,
    MUX_DELETE            = TRACE_MUX_FLAG | 1,
    MUX_PEND              = TRACE_MUX_FLAG | 2,
    MUX_POST              = TRACE_MUX_FLAG | 3,
} PERF_TRACE_TYPE;

// Perf max sample filter task number
#define PERF_MAX_FILTER_TSKS                32

// Perf max sample event counter's number
#define PERF_MAX_EVENT                      7

// Perf max backtrace depth
#define PERF_MAX_CALLCHAIN_DEPTH            10

// Perf sample data buffer's water mark 1/N
#define PERF_BUFFER_WATERMARK_ONE_N         2

enum PerfStatus {
    PERF_UNINIT,
    PERF_STARTED,
    PERF_STOPED,
};

// Define the type of the perf sample data buffer water mark hook function
typedef void (*PERF_BUF_NOTIFY_HOOK)(void);

// Define the type of the perf sample data buffer flush hook function
typedef void (*PERF_BUF_FLUSH_HOOK)(void *addr, U32 size);

/*
 * Perf error code: Bad status.
 *
 * Value: 0x02001700
 *
 * Solution: Follow the perf state machine.
 */
#define OS_ERRNO_PERF_STATUS_INVALID        OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x00)

/*
 * Perf error code: Hardware pmu init failed.
 *
 * Value: 0x02001701
 *
 * Solution: Check the pmu hwi irq.
 */
#define OS_ERRNO_PERF_HW_INIT_ERROR         OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x01)

/*
 * Perf error code: Hrtimer init failed for hrtimer timed pmu init.
 *
 * Value: 0x02001702
 *
 * Solution: Check the Hrtimer init.
 */
#define OS_ERRNO_PERF_TIMED_INIT_ERROR      OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x02)

/*
 * Perf error code: Software pmu init failed.
 *
 * Value: 0x02001703
 *
 * Solution: Check the Perf software events init.
 */
#define OS_ERRNO_PERF_SW_INIT_ERROR         OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x03)

/*
 * Perf error code: Perf buffer init failed.
 *
 * Value: 0x02001704
 *
 * Solution: Check the buffer init size.
 */
#define OS_ERRNO_PERF_BUF_ERROR             OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x04)

/*
 * Perf error code: Perf pmu type error.
 *
 * Value: 0x02001705
 *
 * Solution: Check whether the corresponding pmu is enabled in the menuconfig.
 */
#define OS_ERRNO_PERF_INVALID_PMU           OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x05)

/*
 * Perf error code: Perf pmu config error.
 *
 * Value: 0x02001706
 *
 * Solution: Check the config attr of event id and event period.
 */
#define OS_ERRNO_PERF_PMU_CONFIG_ERROR      OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x06)

/*
 * Perf error code: Perf pmu config attr is null.
 *
 * Value: 0x02001707
 *
 * Solution: Check if the input params of attr is null.
 */
#define OS_ERRNO_PERF_CONFIG_NULL      OS_ERRNO_BUILD_ERROR(OS_MID_PERF, 0x07)

typedef enum {
    PERF_EVENT_TYPE_HW,      // boards common hw events
    PERF_EVENT_TYPE_TIMED,   // hrtimer timed events
    PERF_EVENT_TYPE_SW,      // software trace events
    PERF_EVENT_TYPE_RAW,     // boards special hw events, see enum PmuEventType in corresponding arch headfile

    PERF_EVENT_TYPE_MAX
} PerfEventType;

/*
 * Common hardware pmu events
 */
enum PmuHwId {
    PERF_COUNT_HW_CPU_CYCLES = 0,      // cpu cycle event
    PERF_COUNT_HW_INSTRUCTIONS,        // instruction event
    PERF_COUNT_HW_DCACHE_REFERENCES,   // dcache access event
    PERF_COUNT_HW_DCACHE_MISSES,       // dcache miss event
    PERF_COUNT_HW_ICACHE_REFERENCES,   // icache access event
    PERF_COUNT_HW_ICACHE_MISSES,       // icache miss event
    PERF_COUNT_HW_BRANCH_INSTRUCTIONS, // software change of pc event
    PERF_COUNT_HW_BRANCH_MISSES,       // branch miss event

    PERF_COUNT_HW_MAX,
};

/*
 * Common hrtimer timed events
 */
enum PmuTimedId {
    PERF_COUNT_CPU_CLOCK = 0,      // hrtimer timed event
};

/*
 * Common software pmu events
 */
enum PmuSwId {
    PERF_COUNT_SW_TASK_SWITCH = 1, // task switch event
    PERF_COUNT_SW_HWI_RESPONSE_IN, // irq response in event
    PERF_COUNT_SW_MEM_ALLOC,       // memory alloc event
    PERF_COUNT_SW_SEM_PEND,        // semaphore pend event

    PERF_COUNT_SW_MAX,
};

/*
 * perf sample data types
 * Config it through PerfConfigAttr->sampleType.
 */
typedef enum {
    PERF_RECORD_CPU       = 1U << 0, // record current cpuid
    PERF_RECORD_TID       = 1U << 1, // record current task id
    PERF_RECORD_TYPE      = 1U << 2, // record event type
    PERF_RECORD_PERIOD    = 1U << 3, // record event period
    PERF_RECORD_TIMESTAMP = 1U << 4, // record timestamp
    PERF_RECORD_IP        = 1U << 5, // record instruction pointer
    PERF_RECORD_CALLCHAIN = 1U << 6, // record backtrace
} PerfSampleType;

/*
 * perf configuration sub event information
 *
 * This structure is used to config specific events attributes.
 */
typedef struct {
    PerfEventType type;
    struct {
        U32 eventId;          // the specific event corresponds to the PerfEventType
        U32 period;           // event period, for every "period"th occurrence of the event a sample will be recorded
    } events[PERF_MAX_EVENT]; // perf event list
    U32 eventsNr;             // total perf event number
    bool predivided;          // whether to prescaler (once every 64 counts), which only take effect on cpu cycle hardware event
} PerfEventConfig;

/**
 * perf configuration main information
 *
 * This structure is used to set perf sampling attributes, including events, tasks and other information.
 */
typedef struct {
    PerfEventConfig      eventsCfg;                      // perf event config
    U32                  taskIds[PERF_MAX_FILTER_TSKS];  // perf task filter list (whitelist)
    U32                  taskIdsNr;                      // task numbers of task filter whiltelist, if set 0 perf will sample all tasks
    PerfSampleType       sampleType;                     // type of data to sample
    bool                 needStoreToBuffer;              // whether store to buffer
    bool                 taskFilterEnable;               // whether to filter tasks
} PerfConfigAttr;

extern void OsPerfHook(U32 event);

#if defined(OS_OPTION_PERF) && defined(OS_OPTION_PERF_SW_PMU)
#define PRT_PERF(EVENT) do {      \
        OsPerfHook(EVENT);        \
    } while (0)
#else
#define PRT_PERF(EVENT)
#endif

/**
 * @brief Init perf.
 *
 * @par Description:
 * <ul>
 * <li>Used to initialize the perf module, including initializing the PMU, allocating memory,
 * etc.,which is called during the phase of system initialization.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>If buf is not null, user must ensure size is not bigger than buf's length.</li>
 * </ul>
 *
 * @param  buf     [IN] Pointer of sample data buffer;Use the dynamically allocated memory if the pointer is NULL.
 * @param  size    [IN] Length of sample data buffer;
 *
 * @retval #OS_ERRNO_PERF_STATUS_INVALID              Perf in a wrong status.
 * @retval #OS_ERRNO_PERF_HW_INIT_ERROR               Perf hardware pmu init fail.
 * @retval #OS_ERRNO_PERF_TIMED_INIT_ERROR            Perf timed pmu init fail.
 * @retval #OS_ERRNO_PERF_SW_INIT_ERROR               Perf software pmu init fail.
 * @retval #OS_ERRNO_PERF_BUF_ERROR                   Perf buffer init fail.
 * @retval #OS_OK                                     Perf init success.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
U32 PRT_PerfInit(void *buf, U32 size);

/**
 * @brief Start perf sampling.
 *
 * @par Description
 * Start perf sampling.
 * @attention
 * None.
 *
 * @param  sectionId          [IN] Set the section id for marking this piece of data in the perf sample data buffer.
 * @retval None.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
void PRT_PerfStart(U32 sectionId);

/**
 * @brief Stop perf sampling.
 *
 * @par Description
 * Stop perf sampling.
 * @attention
 * None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
void PRT_PerfStop(void);

/**
 * @brief Config perf parameters.
 *
 * @par Description
 * Config perf parameters before sample, for example, sample event, sample task, etc. it need to be called
 * before PRT_PerfStart.
 * @attention
 * None.
 *
 * @param  attr                      [IN] Address of a perf event attr struct.
 *
 * @retval #OS_ERRNO_PERF_STATUS_INVALID          Perf in a wrong status.
 * @retval #OS_ERRNO_PERF_CONFIG_NULL             Attr is null.
 * @retval #OS_ERRNO_PERF_INVALID_PMU             Config perf pmu with error type.
 * @retval #OS_ERRNO_PERF_PMU_CONFIG_ERROR        Config perf events fail with invaild event id or event period.
 * @retval #OS_OK                                 Config success.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
U32 PRT_PerfConfig(PerfConfigAttr *attr);

/**
 * @brief Read data from perf sample data buffer.
 *
 * @par Description
 * Because perf sample data buffer is a ringbuffer, the data may be covered after user read ringbuffer.
 * @attention
 * None.
 *
 * @param  dest                      [IN] The destionation address.
 * @param  size                      [IN] Read size.
 * @retval #U32                   The really read bytes.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
U32 PRT_PerfDataRead(char *dest, U32 size);

/**
 * @ingroup prt_perf
 * @brief Register perf sample data buffer water mark hook function.
 *
 * @par Description
 * <ul>
 * <li> Register perf sample data buffer water mark hook function.</li>
 * <li> The registered hook will be called when buffer reaches the water mark./li>
 * </ul>
 * @attention
 * None.
 *
 * @param  func                      [IN] Buffer water mark hook function.
 *
 * @retval None.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
void PRT_PerfNotifyHookReg(const PERF_BUF_NOTIFY_HOOK func);

/**
 * @brief Register perf sample data buffer flush hook function.
 *
 * @par Description
 * <ul>
 * <li> Register perf sample data buffer flush hook function.</li>
 * <li> The flush hook will be called when the buffer be read or written.</li>
 * </ul>
 * @attention
 * None.
 *
 * @param  func                      [IN] Buffer flush hook function.
 *
 * @retval None.
 * @par Dependency:
 * <ul>
 * <li>prt_perf.h: the header file that contains the API declaration.</li>
 * </ul>
 * @since Huawei LiteOS V200R005C00
 */
void PRT_PerfFlushHookReg(const PERF_BUF_FLUSH_HOOK func);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* PRT_PERF_H */