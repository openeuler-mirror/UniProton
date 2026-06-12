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
 * Description: Trace模块内部头文件。
 */
#ifndef PRT_TRACE_INTERNAL_H
#define PRT_TRACE_INTERNAL_H

#include "prt_buildef.h"
#include "prt_trace_external.h"
#include "prt_task_external.h"
#include "prt_hwi.h"
#include "prt_lib_external.h"
#include "prt_mem_external.h"
#include "prt_hook_external.h"
#include "prt_clk.h"
#include "prt_sys_external.h"

#ifdef OS_OPTION_TRACE

#define TRACE_MODE_OFFLINE 0
#define TRACE_MODE_ONLINE  1

#define TRACE_DEFAULT_MASK (TRACE_HWI_FLAG | TRACE_TASK_FLAG)
#define TRACE_CTL_MAGIC_NUM 0xDEADBEEFU
#define TRACE_BIGLITTLE_WORD 0x12345678U
#define TRACE_VERSION(MODE)  (0xFFFFFFFFU & (MODE))
#define TRACE_MASK_COMBINE(c1, c2, c3, c4) (((c1) << 24) | ((c2) << 16) | ((c3) << 8) | (c4))
#define TRACE_GET_MODE_FLAG(type) ((type) & 0xFFFFFFF0U)

#define TRACE_LOCK(state)    (state) = PRT_HwiLock()
#define TRACE_UNLOCK(state)  PRT_HwiRestore(state)

typedef void (*TraceDumpHook)(bool toClient);
extern TraceDumpHook g_traceDumpHook;

typedef struct {
    struct WriteCtrl {
        U16 curIndex;
        U16 maxRecordCount;
        U16 curObjIndex;
        U16 maxObjCount;
        TraceObjData *objBuf;
        TraceEventFrame *frameBuf;
    } ctrl;
    TraceOfflineHead *head;
} TraceOfflineHeaderInfo;

extern U32 OsTraceInit(void);
extern U32 OsTraceGetMaskTid(U32 taskId);
extern void OsTraceSetObj(TraceObjData *obj, const struct TagTskCb *tcb);
extern void OsTraceWriteOrSendEvent(const TraceEventFrame *frame);
extern void OsTraceObjAdd(U32 eventType, U32 taskId);
extern bool OsTraceIsEnable(void);
extern TraceOfflineHead *OsTraceRecordGet(void);
extern void OsTraceReset(void);
extern void OsTraceRecordDump(bool toClient);
extern U32 OsTraceBufInit(U32 size);
extern void OsTraceCnvInit(void);

#endif /* OS_OPTION_TRACE */

#endif /* PRT_TRACE_INTERNAL_H */