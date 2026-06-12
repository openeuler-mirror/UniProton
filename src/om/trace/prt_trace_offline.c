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
 * Description: Trace离线模式实现。
 */
#include "prt_trace_internal.h"
#include "prt_mem_external.h"
#include "securec.h"

#ifdef OS_OPTION_TRACE

#define BITS_NUM_FOR_TASK_ID 16

OS_SEC_BSS static TraceOfflineHeaderInfo g_traceRecoder;
OS_SEC_BSS static U32 *g_tidMask = NULL;
OS_SEC_BSS static U32 g_tidMaskSize = 0;

U32 OsTraceGetMaskTid(U32 tid)
{
    if (g_tidMask != NULL && tid < g_tidMaskSize) {
        return tid | (g_tidMask[tid] << BITS_NUM_FOR_TASK_ID);
    }
    return tid;
}

U32 OsTraceBufInit(U32 size)
{
    U32 headSize;
    void *buf = NULL;
    headSize = sizeof(TraceOfflineHead) + sizeof(TraceObjData) * OS_TRACE_OBJ_MAX_NUM;
    if (size <= headSize) {
        return OS_ERRNO_TRACE_BUF_TOO_SMALL;
    }

    buf = PRT_MemAlloc(OS_MID_TRACE, OS_MEM_DEFAULT_FSC_PT, size);
    if (buf == NULL) {
        return OS_ERRNO_TRACE_NO_MEMORY;
    }

    g_tidMaskSize = g_tskMaxNum + 1;
    g_tidMask = (U32 *)PRT_MemAlloc(OS_MID_TRACE, OS_MEM_DEFAULT_FSC_PT, g_tidMaskSize * sizeof(U32));
    if (g_tidMask == NULL) {
        PRT_MemFree(OS_MID_TRACE, buf);
        return OS_ERRNO_TRACE_NO_MEMORY;
    }

    (void)memset_s(buf, size, 0, size);
    (void)memset_s(g_tidMask, g_tidMaskSize * sizeof(U32), 0, g_tidMaskSize * sizeof(U32));
    g_traceRecoder.head = (TraceOfflineHead *)buf;
    g_traceRecoder.head->baseInfo.bigLittleEndian = TRACE_BIGLITTLE_WORD;
    g_traceRecoder.head->baseInfo.version         = TRACE_VERSION(TRACE_MODE_OFFLINE);
    g_traceRecoder.head->baseInfo.clockFreq       = g_systemClock;
    g_traceRecoder.head->objSize                  = sizeof(TraceObjData);
    g_traceRecoder.head->frameSize                = sizeof(TraceEventFrame);
    g_traceRecoder.head->objOffset                = sizeof(TraceOfflineHead);
    g_traceRecoder.head->frameOffset              = headSize;
    g_traceRecoder.head->totalLen                 = (U16)size;

    g_traceRecoder.ctrl.curIndex       = 0;
    g_traceRecoder.ctrl.curObjIndex    = 0;
    g_traceRecoder.ctrl.maxObjCount    = OS_TRACE_OBJ_MAX_NUM;
    g_traceRecoder.ctrl.maxRecordCount = (U16)((size - headSize) / sizeof(TraceEventFrame));
    g_traceRecoder.ctrl.objBuf         = (TraceObjData *)((uintptr_t)buf + g_traceRecoder.head->objOffset);
    g_traceRecoder.ctrl.frameBuf       = (TraceEventFrame *)((uintptr_t)buf + g_traceRecoder.head->frameOffset);

    return OS_OK;
}

void OsTraceObjAdd(U32 eventType, U32 taskId)
{
    uintptr_t intSave;
    U32 index;
    TraceObjData *obj = NULL;

    TRACE_LOCK(intSave);
    index = g_traceRecoder.ctrl.curObjIndex;
    if (index >= OS_TRACE_OBJ_MAX_NUM) {
        TRACE_UNLOCK(intSave);
        return;
    }
    obj = &g_traceRecoder.ctrl.objBuf[index];

    if (g_tidMask != NULL && taskId < g_tidMaskSize) {
        g_tidMask[taskId]++;
    }

    OsTraceSetObj(obj, GET_TCB_HANDLE(taskId));

    g_traceRecoder.ctrl.curObjIndex++;
    if (g_traceRecoder.ctrl.curObjIndex >= g_traceRecoder.ctrl.maxObjCount) {
        g_traceRecoder.ctrl.curObjIndex = 0;
    }
    TRACE_UNLOCK(intSave);
}

void OsTraceWriteOrSendEvent(const TraceEventFrame *frame)
{
    U16 index;
    uintptr_t intSave;

    TRACE_LOCK(intSave);
    index = g_traceRecoder.ctrl.curIndex;
    (void)memcpy_s(&g_traceRecoder.ctrl.frameBuf[index], sizeof(TraceEventFrame),
        frame, sizeof(TraceEventFrame));

    g_traceRecoder.ctrl.curIndex++;
    if (g_traceRecoder.ctrl.curIndex >= g_traceRecoder.ctrl.maxRecordCount) {
        g_traceRecoder.ctrl.curIndex = 0;
    }
    TRACE_UNLOCK(intSave);
}

void OsTraceReset(void)
{
    uintptr_t intSave;
    U32 bufLen;

    TRACE_LOCK(intSave);
    bufLen = sizeof(TraceEventFrame) * g_traceRecoder.ctrl.maxRecordCount;
    (void)memset_s(g_traceRecoder.ctrl.frameBuf, bufLen, 0, bufLen);
    g_traceRecoder.ctrl.curIndex = 0;
    TRACE_UNLOCK(intSave);
}

extern U32 PRT_Printf(const char *format, ...);

static void OsTraceInfoObj(void)
{
    U32 i;
    TraceObjData *obj = &g_traceRecoder.ctrl.objBuf[0];

    if (g_traceRecoder.ctrl.maxObjCount > 0) {
        PRT_Printf("CurObjIndex = %u\n", g_traceRecoder.ctrl.curObjIndex);
        PRT_Printf("Index   TaskID   TaskPrio   TaskName \n");
        for (i = 0; i < g_traceRecoder.ctrl.maxObjCount; i++, obj++) {
            PRT_Printf("%-7u 0x%-6x %-10u %s\n", i, obj->id, obj->prio, obj->name);
        }
        PRT_Printf("\n");
    }
}

static void OsTraceInfoEventTitle(void)
{
    PRT_Printf("CurEvtIndex = %u\n", g_traceRecoder.ctrl.curIndex);
    PRT_Printf("Index   Time(cycles)      EventType      CurTask   Identity      ");
#ifdef OS_OPTION_TRACE_FRAME_CORE_MSG
    PRT_Printf("cpuId    hwiActive    taskLockCnt    ");
#endif
#ifdef OS_OPTION_TRACE_FRAME_EVENT_COUNT
    PRT_Printf("eventCount    ");
#endif
    if (OS_TRACE_FRAME_MAX_PARAMS > 0) {
        PRT_Printf("params    ");
    }
    PRT_Printf("\n");
}

static void OsTraceInfoEventData(void)
{
    U32 i, j;
    TraceEventFrame *frame = &g_traceRecoder.ctrl.frameBuf[0];

    for (i = 0; i < g_traceRecoder.ctrl.maxRecordCount; i++, frame++) {
        PRT_Printf("%-7u 0x%-15x 0x%-12x 0x%-7x 0x%-11x ",
            i, (U32)frame->curTime, frame->eventType, frame->curTask, frame->identity);
#ifdef OS_OPTION_TRACE_FRAME_CORE_MSG
        PRT_Printf("%-11u %-11u %-11u",
            frame->core.cpuId, frame->core.hwiActive, frame->core.taskLockCnt);
#endif
#ifdef OS_OPTION_TRACE_FRAME_EVENT_COUNT
        PRT_Printf("%-11u", frame->eventCount);
#endif
        for (j = 0; j < OS_TRACE_FRAME_MAX_PARAMS; j++) {
            PRT_Printf("0x%-11x", (U32)frame->params[j]);
        }
        PRT_Printf("\n");
    }
}

static void OsTraceInfoDisplay(void)
{
    TraceOfflineHead *head = g_traceRecoder.head;

    PRT_Printf("*******TraceInfo begin*******\n");
    PRT_Printf("clockFreq = %u\n", head->baseInfo.clockFreq);

    OsTraceInfoObj();
    OsTraceInfoEventTitle();
    OsTraceInfoEventData();

    PRT_Printf("*******TraceInfo end*******\n");
}

void OsTraceRecordDump(bool toClient)
{
    (void)toClient;
    OsTraceInfoDisplay();
}

TraceOfflineHead *OsTraceRecordGet(void)
{
    return g_traceRecoder.head;
}

#endif /* OS_OPTION_TRACE */