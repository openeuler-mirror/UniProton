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
 * Create: 2024-03-06
 * Description: Perf
 */

#ifndef PRT_PERF_OUTPUT_H
#define PRT_PERF_OUTPUT_H

#include "include/prt_perf_comm.h"
#include "prt_ringbuf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct {
    Ringbuf ringbuf;        /* ring buffer */
    U32 waterMark;          /* notify water mark */
} PerfOutputCB;

extern U32 OsPerfOutPutInit(void *buf, U32 size);
extern U32 OsPerfOutPutRead(char *dest, U32 size);
extern U32 OsPerfOutPutWrite(char *data, U32 size);
extern U32 OsPerfOutPutRemainSize();
extern void OsPerfOutPutInfo(void);
extern void OsPerfOutPutFlush(void);
extern void OsPerfNotifyHookReg(const PERF_BUF_NOTIFY_HOOK func);
extern void OsPerfFlushHookReg(const PERF_BUF_FLUSH_HOOK func);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* PRT_PERF_OUTPUT_H */
