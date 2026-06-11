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
 * Create: 2024-06-10
 * Description: Trace模块外部接口头文件。
 */
#ifndef PRT_TRACE_EXTERNAL_H
#define PRT_TRACE_EXTERNAL_H

#include "prt_typedef.h"
#include "prt_errno.h"
#include "prt_module.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef OS_OPTION_TRACE

#define OS_TRACE_OBJ_MAX_NAME_SIZE 16

#ifndef OS_TRACE_OBJ_MAX_NUM
#define OS_TRACE_OBJ_MAX_NUM 0
#endif

#ifndef OS_TRACE_FRAME_MAX_PARAMS
#define OS_TRACE_FRAME_MAX_PARAMS 3
#endif

#define OS_ERRNO_TRACE_ERROR_STATUS OS_ERRNO_BUILD_ERROR(OS_MID_TRACE, 0x00)
#define OS_ERRNO_TRACE_NO_MEMORY    OS_ERRNO_BUILD_ERROR(OS_MID_TRACE, 0x01)
#define OS_ERRNO_TRACE_BUF_TOO_SMALL OS_ERRNO_BUILD_ERROR(OS_MID_TRACE, 0x02)

enum TraceState {
    TRACE_UNINIT = 0,
    TRACE_INITED,
    TRACE_STARTED,
    TRACE_STOPED,
};

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
} TraceMask;

typedef enum {
    SYS_ERROR             = TRACE_SYS_FLAG | 0,
    SYS_START             = TRACE_SYS_FLAG | 1,
    SYS_STOP              = TRACE_SYS_FLAG | 2,

    HWI_CREATE              = TRACE_HWI_FLAG | 0,
    HWI_DELETE              = TRACE_HWI_FLAG | 2,
    HWI_RESPONSE_IN         = TRACE_HWI_FLAG | 4,
    HWI_RESPONSE_OUT        = TRACE_HWI_FLAG | 5,
    HWI_ENABLE              = TRACE_HWI_FLAG | 6,
    HWI_DISABLE             = TRACE_HWI_FLAG | 7,

    TASK_CREATE           = TRACE_TASK_FLAG | 0,
    TASK_PRIOSET          = TRACE_TASK_FLAG | 1,
    TASK_DELETE           = TRACE_TASK_FLAG | 2,
    TASK_SUSPEND          = TRACE_TASK_FLAG | 3,
    TASK_RESUME           = TRACE_TASK_FLAG | 4,
    TASK_SWITCH           = TRACE_TASK_FLAG | 5,

    SWTMR_CREATE          = TRACE_SWTMR_FLAG | 0,
    SWTMR_DELETE          = TRACE_SWTMR_FLAG | 1,
    SWTMR_START           = TRACE_SWTMR_FLAG | 2,
    SWTMR_STOP            = TRACE_SWTMR_FLAG | 3,
    SWTMR_EXPIRED         = TRACE_SWTMR_FLAG | 4,

    MEM_ALLOC             = TRACE_MEM_FLAG | 0,
    MEM_ALLOC_ALIGN       = TRACE_MEM_FLAG | 1,
    MEM_REALLOC           = TRACE_MEM_FLAG | 2,
    MEM_FREE              = TRACE_MEM_FLAG | 3,
    MEM_INFO_REQ          = TRACE_MEM_FLAG | 4,
    MEM_INFO              = TRACE_MEM_FLAG | 5,

    QUEUE_CREATE          = TRACE_QUE_FLAG | 0,
    QUEUE_DELETE          = TRACE_QUE_FLAG | 1,
    QUEUE_RW              = TRACE_QUE_FLAG | 2,

    EVENT_CREATE          = TRACE_EVENT_FLAG | 0,
    EVENT_DELETE          = TRACE_EVENT_FLAG | 1,
    EVENT_READ            = TRACE_EVENT_FLAG | 2,
    EVENT_WRITE           = TRACE_EVENT_FLAG | 3,
    EVENT_CLEAR           = TRACE_EVENT_FLAG | 4,

    SEM_CREATE            = TRACE_SEM_FLAG | 0,
    SEM_DELETE            = TRACE_SEM_FLAG | 1,
    SEM_PEND              = TRACE_SEM_FLAG | 2,
    SEM_POST              = TRACE_SEM_FLAG | 3,

    MUX_CREATE            = TRACE_MUX_FLAG | 0,
    MUX_DELETE            = TRACE_MUX_FLAG | 1,
    MUX_PEND              = TRACE_MUX_FLAG | 2,
    MUX_POST              = TRACE_MUX_FLAG | 3,
} TraceEventType;

typedef struct {
    U32 bigLittleEndian;
    U32 clockFreq;
    U32 version;
} TraceBaseHeaderInfo;

typedef struct {
    U32  eventType;
    U32  curTask;
    U64  curTime;
    uintptr_t identity;
#ifdef OS_OPTION_TRACE_FRAME_CORE_MSG
    struct CoreStatus {
        U32 cpuId      : 8,
               hwiActive  : 4,
               taskLockCnt : 4,
               paramCount : 4,
               reserves   : 12;
    } core;
#endif

#ifdef OS_OPTION_TRACE_FRAME_EVENT_COUNT
    U32  eventCount;
#endif

    uintptr_t params[OS_TRACE_FRAME_MAX_PARAMS];
} TraceEventFrame;

typedef struct {
    U32      id;
    U32      prio;
    char     name[OS_TRACE_OBJ_MAX_NAME_SIZE];
} TraceObjData;

typedef struct {
    TraceBaseHeaderInfo baseInfo;
    U16 totalLen;
    U16 objSize;
    U16 frameSize;
    U16 objOffset;
    U16 frameOffset;
} TraceOfflineHead;

typedef bool (*TraceHwiFilterHook)(U32 hwiNum);
typedef void (*TraceEventHook)(U32 eventType, uintptr_t identity, const uintptr_t *params, U16 paramCount);
extern TraceEventHook g_traceEventHook;

#define TASK_SWITCH_PARAMS(taskId, oldPriority, oldTaskStatus, newPriority, newTaskStatus) \
    taskId, oldPriority, oldTaskStatus, newPriority, newTaskStatus
#define TASK_PRIOSET_PARAMS(taskId, taskStatus, oldPrio, newPrio) taskId, taskStatus, oldPrio, newPrio
#define TASK_CREATE_PARAMS(taskId, taskStatus, prio)     taskId, taskStatus, prio
#define TASK_DELETE_PARAMS(taskId, taskStatus, usrStack) taskId, taskStatus, usrStack
#define TASK_SUSPEND_PARAMS(taskId, taskStatus, runTaskId) taskId, taskStatus, runTaskId
#define TASK_RESUME_PARAMS(taskId, taskStatus, prio)     taskId, taskStatus, prio

#define SWTMR_START_PARAMS(swtmrId, mode, interval)  swtmrId, mode, interval
#define SWTMR_DELETE_PARAMS(swtmrId)                                  swtmrId
#define SWTMR_EXPIRED_PARAMS(swtmrId)                                 swtmrId
#define SWTMR_STOP_PARAMS(swtmrId)                                    swtmrId
#define SWTMR_CREATE_PARAMS(swtmrId)                                  swtmrId

#define HWI_CREATE_PARAMS(hwiNum, hwiPrio, hwiMode, hwiHandler) hwiNum, hwiPrio, hwiMode, hwiHandler
#define HWI_DELETE_PARAMS(hwiNum)                       hwiNum
#define HWI_RESPONSE_IN_PARAMS(hwiNum)                  hwiNum
#define HWI_RESPONSE_OUT_PARAMS(hwiNum)                 hwiNum
#define HWI_ENABLE_PARAMS(hwiNum)                       hwiNum
#define HWI_DISABLE_PARAMS(hwiNum)                      hwiNum

#define EVENT_CREATE_PARAMS(eventCB)                    eventCB
#define EVENT_DELETE_PARAMS(eventCB, delRetCode)        eventCB, delRetCode
#define EVENT_READ_PARAMS(eventCB, eventId, mask, mode, timeout) \
    eventCB, eventId, mask, mode, timeout
#define EVENT_WRITE_PARAMS(eventCB, eventId, events)    eventCB, eventId, events
#define EVENT_CLEAR_PARAMS(eventCB, eventId, events)    eventCB, eventId, events

#define QUEUE_CREATE_PARAMS(queueId, queueSz, itemSz, queueAddr, memType) \
    queueId, queueSz, itemSz, queueAddr, memType
#define QUEUE_DELETE_PARAMS(queueId, state, readable)   queueId, state, readable
#define QUEUE_RW_PARAMS(queueId, queueSize, bufSize, operateType, readable, writable, timeout) \
    queueId, queueSize, bufSize, operateType, readable, writable, timeout

#define SEM_CREATE_PARAMS(semId, type, count)           semId, type, count
#define SEM_DELETE_PARAMS(semId, delRetCode)            semId, delRetCode
#define SEM_PEND_PARAMS(semId, count, timeout)          semId, count, timeout
#define SEM_POST_PARAMS(semId, type, count)             semId, type, count

#define MUX_CREATE_PARAMS(muxId)                        muxId
#define MUX_DELETE_PARAMS(muxId, state, count, owner)   muxId, state, count, owner
#define MUX_PEND_PARAMS(muxId, count, owner, timeout)   muxId, count, owner, timeout
#define MUX_POST_PARAMS(muxId, count, owner)            muxId, count, owner

#define MEM_ALLOC_PARAMS(pool, ptr, size)                   pool, ptr, size
#define MEM_ALLOC_ALIGN_PARAMS(pool, ptr, size, boundary)   pool, ptr, size, boundary
#define MEM_REALLOC_PARAMS(pool, ptr, size)                 pool, ptr, size
#define MEM_FREE_PARAMS(pool, ptr)                          pool, ptr
#define MEM_INFO_REQ_PARAMS(pool)                           pool
#define MEM_INFO_PARAMS(pool, usedSize, freeSize)           pool, usedSize, freeSize

#define SYS_ERROR_PARAMS(errno)                         errno

#define PRT_TRACE(TYPE, IDENTITY, ...)                                             \
    do {                                                                           \
        uintptr_t _inner[] = {0, TYPE##_PARAMS((uintptr_t)IDENTITY, ##__VA_ARGS__)};   \
        U32 _n = sizeof(_inner) / sizeof(uintptr_t);                               \
        if ((_n > 1) && (g_traceEventHook != NULL)) {                              \
            g_traceEventHook(TYPE, _inner[1], _n > 2 ? &_inner[2] : NULL, _n - 2); \
        }                                                                          \
    } while (0)

#define PRT_TRACE_EASY(TYPE, IDENTITY, ...)                                                                          \
    do {                                                                                                             \
        uintptr_t _inner[] = {0, ##__VA_ARGS__};                                                                       \
        U32 _n = sizeof(_inner) / sizeof(uintptr_t);                                                               \
        if (g_traceEventHook != NULL) {                                                                              \
            g_traceEventHook(TRACE_USER_DEFAULT_FLAG | TYPE, (uintptr_t)IDENTITY, _n > 1 ? &_inner[1] : NULL, _n - 1); \
        }                                                                                                            \
    } while (0)

extern U32 PRT_TraceStart(void);
extern void PRT_TraceStop(void);
extern void PRT_TraceReset(void);
extern void PRT_TraceEventMaskSet(U32 mask);
extern void PRT_TraceRecordDump(bool toClient);
extern TraceOfflineHead *PRT_TraceRecordGet(void);
extern void PRT_TraceHwiFilterHookReg(TraceHwiFilterHook hook);

#else /* !OS_OPTION_TRACE */

#define PRT_TRACE(TYPE, ...)
#define PRT_TRACE_EASY(TYPE, ...)

#endif /* OS_OPTION_TRACE */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_TRACE_EXTERNAL_H */