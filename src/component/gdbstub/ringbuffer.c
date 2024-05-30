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

#include <string.h>
#include "prt_typedef.h"
#include "gdbstub_common.h"

#define LINE_BUF_SIZE 128

typedef struct RingBuffer {
    unsigned int  in;
    unsigned int  out;
    unsigned int  len;
    unsigned int  reserved;
    char          data[0];
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

#ifdef __x86_64__
#define WMB()   __asm__ volatile("sfence" ::: "memory")
#elif __aarch64__
#define DSB(opt) __asm__ volatile("dsb " #opt : : : "memory")
#define WMB()       DSB(st)
#else
#error  "unsupported arch"
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
/*
 * internal helper to calculate the unused elements in a fifo
 */
static inline unsigned int FifoUnused(struct RingBuffer *fifo)
{
    return fifo->len - (fifo->in - fifo->out);
}

static STUB_TEXT void FifoCopyIn(struct RingBuffer *fifo, const void *src, unsigned int len, unsigned int off)
{
    unsigned int size = fifo->len;
    unsigned int l;

    off = off % size;
    l = MIN(len, size - off);

    memcpy(fifo->data + off, src, l);
    memcpy(fifo->data, src + l, len - l);
    /*
     * make sure that the data in the fifo is up to date before
     * incrementing the fifo->in index counter
     */
    WMB();
}

STUB_TEXT unsigned int FifoIn(struct RingBuffer *fifo, const void *buf, unsigned int len)
{
    unsigned int l;

    l = FifoUnused(fifo);
    if (len > l)
        len = l;

    FifoCopyIn(fifo, buf, len, fifo->in);
    fifo->in += len;
    return len;
}

static STUB_TEXT void FifoCopyOut(struct RingBuffer *fifo, void *dst, unsigned int len, unsigned int off)
{
    unsigned int size = fifo->len;
    unsigned int l;

    off %= size;
    l = MIN(len, size - off);

    memcpy(dst, fifo->data + off, l);
    memcpy(dst + l, fifo->data, len - l);
    /*
     * make sure that the data is copied before
     * incrementing the fifo->out index counter
     */
    WMB();
}

static STUB_TEXT unsigned int FifoOutPeek(struct RingBuffer *fifo, void *buf, unsigned int len)
{
    unsigned int l;

    l = fifo->in - fifo->out;
    if (len > l)
        len = l;

    FifoCopyOut(fifo, buf, len, fifo->out);
    return len;
}

static STUB_TEXT unsigned int FifoOut(struct RingBuffer *fifo, void *buf, unsigned int len)
{
    len = FifoOutPeek(fifo, buf, len);
    fifo->out += len;
    return len;
}

STUB_TEXT int RBufferPairInit(uintptr_t rxaddr, uintptr_t txaddr, int len)
{
    if (!rxaddr || !txaddr || len <= sizeof(RBuffer)) {
        return -1;
    }

    g_rxRBuffer = (RBuffer *)rxaddr;
    g_txRBuffer = (RBuffer *)txaddr;
    return 0;
}

static STUB_TEXT int OsRbufferWrite(RBuffer *buffer, char *buf, int len)
{
    int cnt = 0;

    if (len <= 0) {
        return 0;
    }
    while (cnt < len) {
        int o = FifoIn(buffer, &buf[cnt], len - cnt);
        cnt += o;
    }
 
    return cnt;
}

static STUB_TEXT int RBufferRead(RBuffer *buffer, char *buf, int len)
{
    return FifoOut(buffer, buf, len);
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
extern struct GdbRingBufferCfg *OsGetGdbRingBufferCfg(void);

STUB_TEXT int OsGdbRingBufferInit()
{
    struct GdbRingBufferCfg *cfg = OsGetGdbRingBufferCfg();
    if (!cfg) {
        return -1;
    }
    RBufferPairInit(cfg->rxaddr, cfg->txaddr, cfg->size);
    return 0;
}
