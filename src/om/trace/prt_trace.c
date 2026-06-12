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
 * Description: Trace模块核心实现。
 */
#include "prt_trace_internal.h"

#ifdef OS_OPTION_TRACE

#define EVENT_MASK 0xFFFFFFF0U

OS_SEC_BSS static U32 g_traceEventCount;
OS_SEC_BSS static volatile enum TraceState g_traceState = TRACE_UNINIT;
OS_SEC_DATA static volatile bool g_enableTrace = FALSE;
OS_SEC_BSS static U32 g_traceMask = TRACE_DEFAULT_MASK;

TraceEventHook g_traceEventHook = NULL;
TraceDumpHook g_traceDumpHook = NULL;

OS_SEC_BSS static TraceHwiFilterHook g_traceHwiFliterHook = NULL;

static bool OsTraceHwiFilter(U32 hwiNum)
{
    bool ret = FALSE;
    if (g_traceHwiFliterHook != NULL) {
        ret = g_traceHwiFliterHook(hwiNum);
    }
    return ret;
}

static void OsTraceSetFrame(TraceEventFrame *frame, U32 eventType, uintptr_t identity,
    const uintptr_t *params, U16 paramCount)
{
    U32 i;
    uintptr_t intSave;

    (void)memset_s(frame, sizeof(TraceEventFrame), 0, sizeof(TraceEventFrame));

    if (paramCount > OS_TRACE_FRAME_MAX_PARAMS) {
        paramCount = OS_TRACE_FRAME_MAX_PARAMS;
    }

    TRACE_LOCK(intSave);
    frame->curTask   = OsTraceGetMaskTid(RUNNING_TASK->taskPid);
    frame->identity  = identity;
    frame->curTime   = PRT_ClkGetCycleCount64();
    frame->eventType = eventType;

#ifdef OS_OPTION_TRACE_FRAME_CORE_MSG
    frame->core.cpuId      = 0;
    frame->core.hwiActive  = 0;
    frame->core.taskLockCnt = 0;
    frame->core.paramCount = paramCount;
#endif

#ifdef OS_OPTION_TRACE_FRAME_EVENT_COUNT
    frame->eventCount = g_traceEventCount;
    g_traceEventCount++;
#endif
    TRACE_UNLOCK(intSave);

    for (i = 0; i < paramCount; i++) {
        frame->params[i] = params[i];
    }
}

void OsTraceSetObj(TraceObjData *obj, const struct TagTskCb *tcb)
{
    (void)memset_s(obj, sizeof(TraceObjData), 0, sizeof(TraceObjData));

    obj->id   = OsTraceGetMaskTid(tcb->taskPid);
    obj->prio = (U32)tcb->priority;

#ifdef OS_OPTION_TASK_INFO
    U32 nameLen = OS_TRACE_OBJ_MAX_NAME_SIZE - 1;
    U32 copyLen = 0;
    while (copyLen < nameLen && tcb->name[copyLen] != '\0') {
        obj->name[copyLen] = tcb->name[copyLen];
        copyLen++;
    }
    obj->name[copyLen] = '\0';
#else
    obj->name[0] = '\0';
#endif
}

void OsTraceHook(U32 eventType, uintptr_t identity, const uintptr_t *params, U16 paramCount)
{
    if ((eventType == TASK_CREATE) || (eventType == TASK_PRIOSET)) {
        OsTraceObjAdd(eventType, identity);
    }

    if ((g_enableTrace == TRUE) && (eventType & g_traceMask)) {
        uintptr_t id = identity;
        if (TRACE_GET_MODE_FLAG(eventType) == TRACE_HWI_FLAG) {
            if (OsTraceHwiFilter(identity)) {
                return;
            }
        } else if (TRACE_GET_MODE_FLAG(eventType) == TRACE_TASK_FLAG) {
            id = OsTraceGetMaskTid(identity);
        }

        TraceEventFrame frame;
        OsTraceSetFrame(&frame, eventType, id, params, paramCount);
        OsTraceWriteOrSendEvent(&frame);
    }
}

bool OsTraceIsEnable(void)
{
    return g_enableTrace == TRUE;
}

static void OsTraceHookInstall(void)
{
    g_traceEventHook = OsTraceHook;
    g_traceDumpHook = OsTraceRecordDump;
}

U32 OsTraceInit(void)
{
    uintptr_t intSave;
    U32 ret;

    TRACE_LOCK(intSave);
    if (g_traceState != TRACE_UNINIT) {
        TRACE_UNLOCK(intSave);
        return OS_ERRNO_TRACE_ERROR_STATUS;
    }

    ret = OsTraceBufInit(OS_TRACE_BUFFER_SIZE);
    if (ret != OS_OK) {
        TRACE_UNLOCK(intSave);
        return ret;
    }

    OsTraceHookInstall();
    OsTraceCnvInit();

    g_traceEventCount = 0;
    g_enableTrace = TRUE;
    g_traceState = TRACE_STARTED;
    TRACE_UNLOCK(intSave);
    return OS_OK;
}

U32 PRT_TraceStart(void)
{
    uintptr_t intSave;

    TRACE_LOCK(intSave);
    if (g_traceState == TRACE_STARTED) {
        TRACE_UNLOCK(intSave);
        return OS_OK;
    }

    if (g_traceState == TRACE_UNINIT) {
        TRACE_UNLOCK(intSave);
        return OS_ERRNO_TRACE_ERROR_STATUS;
    }

    g_enableTrace = TRUE;
    g_traceState = TRACE_STARTED;
    TRACE_UNLOCK(intSave);
    return OS_OK;
}

void PRT_TraceStop(void)
{
    uintptr_t intSave;

    TRACE_LOCK(intSave);
    if (g_traceState != TRACE_STARTED) {
        TRACE_UNLOCK(intSave);
        return;
    }

    g_enableTrace = FALSE;
    g_traceState = TRACE_STOPED;
    TRACE_UNLOCK(intSave);
}

void PRT_TraceEventMaskSet(U32 mask)
{
    g_traceMask = mask & EVENT_MASK;
}

void PRT_TraceRecordDump(bool toClient)
{
    if (g_traceState != TRACE_STOPED) {
        return;
    }
    OsTraceRecordDump(toClient);
}

TraceOfflineHead *PRT_TraceRecordGet(void)
{
    return OsTraceRecordGet();
}

void PRT_TraceReset(void)
{
    if (g_traceState == TRACE_UNINIT) {
        return;
    }
    OsTraceReset();
}

void PRT_TraceHwiFilterHookReg(TraceHwiFilterHook hook)
{
    uintptr_t intSave;
    TRACE_LOCK(intSave);
    g_traceHwiFliterHook = hook;
    TRACE_UNLOCK(intSave);
}

#endif /* OS_OPTION_TRACE */