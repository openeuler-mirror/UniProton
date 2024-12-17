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
 * Create: 2024-03-14
 */

#include "prt_perf_output.h"

OS_SEC_BSS static PERF_BUF_NOTIFY_HOOK g_perfBufNotifyHook;
OS_SEC_BSS static PERF_BUF_FLUSH_HOOK g_perfBufFlushHook;
OS_SEC_BSS static PerfOutputCB g_perfOutputCb;

static void OsPerfDefaultNotify(void)
{
    printf("perf buf waterline notify!\n");
}

U32 OsPerfOutPutInit(void *buf, U32 size)
{
    U32 ret;
    bool releaseFlag = FALSE;

    if (PERF_BUFFER_WATERMARK_ONE_N == 0) {
        printf("perf buffer watermark configure failed\n");
        return OS_ERROR;
    }

    if (buf == NULL) {
        buf = PRT_MemAlloc(OS_MID_PERF, OS_MEM_DEFAULT_PT0, size);
        if (buf == NULL) {
            printf("perf malloc output buffer failed\n");
            return OS_ERROR;
        }

        releaseFlag = TRUE;
        (void)memset_s(buf, size, 0, size);
    }

    ret = PRT_RingbufInit(&g_perfOutputCb.ringbuf, buf, size);
    if (ret != OS_OK) {
        goto RELEASE;
    }

    g_perfOutputCb.waterMark = size / PERF_BUFFER_WATERMARK_ONE_N;
    g_perfBufNotifyHook = OsPerfDefaultNotify;
    return ret;
RELEASE:
    if (releaseFlag) {
        (void)PRT_MemFree(OS_MID_PERF, buf);
    }

    return ret;
}

void OsPerfOutPutFlush(void)
{
    if (g_perfBufFlushHook != NULL) {
        g_perfBufFlushHook(g_perfOutputCb.ringbuf.fifo, g_perfOutputCb.ringbuf.size);
    }

    return;
}

U32 OsPerfOutPutRead(char *dest, U32 size)
{
    OsPerfOutPutFlush();
    return PRT_RingbufRead(&g_perfOutputCb.ringbuf, dest, size);
}

static int OsPerfOutPutBegin(U32 size)
{
    if (g_perfOutputCb.ringbuf.remain < size) {
        printf("perf buf remain %u, and has no enough space for 0x%x\n", g_perfOutputCb.ringbuf.remain, size);
        return FALSE;
    }

    return TRUE;
}

static void OsPerfOutPutEnd(void)
{
    OsPerfOutPutFlush();
    if (PRT_RingbufUsedSize(&g_perfOutputCb.ringbuf) >= g_perfOutputCb.waterMark) {
        if (g_perfBufNotifyHook != NULL) {
            g_perfBufNotifyHook();
        }
    }

    return;
}

U32 OsPerfOutPutWrite(char *data, U32 size)
{
    if (!OsPerfOutPutBegin(size)) {
        return OS_ERROR;
    }

    PRT_RingbufWrite(&g_perfOutputCb.ringbuf, data, size);

    OsPerfOutPutEnd();
    return OS_OK;
}

U32 OsPerfOutPutRemainSize()
{
    return g_perfOutputCb.ringbuf.remain;
}

void OsPerfOutPutInfo(void)
{
    printf("dump section data, addr: %p length: %#x\n", g_perfOutputCb.ringbuf.fifo, g_perfOutputCb.ringbuf.size);
    return;
}

void OsPerfNotifyHookReg(const PERF_BUF_NOTIFY_HOOK func)
{
    g_perfBufNotifyHook = func;
    return;
}

void OsPerfFlushHookReg(const PERF_BUF_FLUSH_HOOK func)
{
    g_perfBufFlushHook = func;
    return;
}