/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-09-14
 * Description: ringbuffer实现
 */


#include "prt_typedef.h"
#include "gdbstub_common.h"

#define LINE_BUF_SIZE 128
#define REDZONE_SIZE  8

typedef struct RingBuffer {
    int len;
    volatile int busy;
    volatile int tail;
    volatile int head;
    char redzone[REDZONE_SIZE];
} RBuffer;

typedef struct LineBuffer {
    char data[LINE_BUF_SIZE];
    int avails;
    int idx;
} LBuffer;

static STUB_DATA RBuffer *g_rxRBuffer;
static STUB_DATA RBuffer *g_txRBuffer;

static STUB_DATA LBuffer g_txLBuffer;
static STUB_DATA LBuffer g_rxLBuffer;

static STUB_TEXT void RBufferLock(RBuffer *rbuffer)
{
    while (rbuffer->busy) {
        ;
    }
    rbuffer->busy = 1;
}

static STUB_TEXT void RBufferUnlock(RBuffer *rbuffer)
{
    rbuffer->busy = 0;
}

static STUB_TEXT RBuffer * RBufferInit(uintptr_t addr, int len)
{
    RBuffer *buffer = (RBuffer *)addr;
    buffer->busy = 0;
    buffer->len = len;
    buffer->tail = buffer->head = 0;
    for (int i = 0; i < sizeof(buffer->redzone); i++) {
        buffer->redzone[i] = 0x7a;
    }
    return buffer;
}

int STUB_TEXT RBufferPairInit(uintptr_t rxaddr, uintptr_t txaddr, int len)
{
    if (!rxaddr || !txaddr || len <= sizeof(RBuffer)) {
        return -1;
    }

    g_rxRBuffer = RBufferInit(rxaddr, len - sizeof(RBuffer));
    g_txRBuffer = RBufferInit(txaddr, len - sizeof(RBuffer));
    return 0;
}

INLINE int IsEmpty(RBuffer *buffer)
{
    return buffer->head == buffer->tail;
}

INLINE int IsFull(RBuffer *buffer)
{
    return (buffer->tail + 1) % buffer->len == buffer->head;
}

static STUB_TEXT int OsRbufferWrite(RBuffer *buffer, char *buf, int len)
{
    int dlen = buffer->len;
    int cnt = 0;
    char *data = (char *)buffer + sizeof(RBuffer);

    if (len <= 0) {
        return 0;
    }
    RBufferLock(buffer);
    while (cnt < len) {
        if (IsFull(buffer)) {
            break;
        }
        data[(buffer->tail++) % dlen] = buf[cnt++];
    }
    RBufferUnlock(buffer);
 
    return cnt;
}

static STUB_TEXT int RBufferRead(RBuffer *buffer, char *buf, int len)
{
    int dlen = buffer->len;
    char *data = (char *)buffer + sizeof(RBuffer);
    int cnt = 0;
    RBufferLock(buffer);
    while (cnt < len) {
        if (IsEmpty(buffer)) {
            break;
        }
        buf[cnt++] = data[(buffer->head++) % dlen];
    }
    RBufferUnlock(buffer);

    return cnt;
}

/* Non-thread-safe */
STUB_TEXT void OsGdbFlush()
{
    int len = g_txLBuffer.idx;
    int cnt = 0;
    while (cnt < len) {
        cnt += OsRbufferWrite(g_txRBuffer, &g_txLBuffer.data[cnt], len - cnt);
    }
    g_txLBuffer.idx = 0;
}

/* Non-thread-safe */
STUB_TEXT void OsGdbPutchar(char ch)
{
    int len = sizeof(g_txLBuffer.data);
    g_txLBuffer.data[g_txLBuffer.idx++] = ch;

    if (g_txLBuffer.idx == len) {
        OsGdbFlush();
        g_txLBuffer.idx = 0;
    }
}

/* Non-thread-safe */
STUB_TEXT char OsGdbGetchar()
{
    char ch;
    int len = sizeof(g_rxLBuffer.data);
    if (g_rxLBuffer.idx >= len || g_rxLBuffer.idx >= g_rxLBuffer.avails) {
        g_rxLBuffer.idx = 0;
        int cnt = 0;
        while ((cnt = RBufferRead(g_rxRBuffer, g_rxLBuffer.data, sizeof(g_rxLBuffer.data))) == 0) {
            ;
        }
        g_rxLBuffer.avails = cnt;
    }  
    ch = g_rxLBuffer.data[g_rxLBuffer.idx++];
    return ch;
}

extern struct GdbRingBufferCfg *OsGetGdbRingBufferCfg();

STUB_TEXT int OsGdbIOInit()
{
    struct GdbRingBufferCfg *cfg = OsGetGdbRingBufferCfg();
    if (!cfg) {
        return -1;
    }
    RBufferPairInit(cfg->rxaddr, cfg->txaddr, cfg->size);
    return 0;
}
