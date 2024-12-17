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
 * Create: 2024-03-13
 * Description: RingBuf
 */

#include "prt_ringbuf.h"
#include "prt_cpu_external.h"

U32 PRT_RingbufUsedSize(Ringbuf *ringbuf)
{
    U32 size;
    uintptr_t intSave;

    if ((ringbuf == NULL) || (ringbuf->status != RBUF_INITED)) {
        return 0;
    }

    intSave = PRT_SplIrqLock(&ringbuf->lock);
    size = ringbuf->size - ringbuf->remain;
    PRT_SplIrqUnlock(&ringbuf->lock, intSave);

    return size;
}

/*
 *                    startIdx
 *                    |
 *    0 0 0 0 0 0 0 0 X X X X X X X X 0 0 0 0 0 0
 *                                    |
 *                                  endIdx
 */
static U32 OsRingbufWriteLinear(Ringbuf *ringbuf, const char *buf, U32 size)
{
    U32 cpSize;
    U32 ret;

    cpSize = (ringbuf->remain < size) ? ringbuf->remain : size;

    if (cpSize == 0) {
        return 0;
    }

    ret = memcpy_s(ringbuf->fifo + ringbuf->endIdx, ringbuf->remain, buf, cpSize);
    if (ret != OS_OK) {
        printf("write linear ring buffer failed, endIdx: %u, cpSize=%u\n", ringbuf->endIdx, cpSize);
        return 0;
    }

    ringbuf->remain -= cpSize;
    ringbuf->endIdx += cpSize;

    return cpSize;
}

static U32 OsRingbufWriteLoop(Ringbuf *ringbuf, const char *buf, U32 size)
{
    U32 ret;
    U32 right;
    U32 cpSize;

    right = ringbuf->size - ringbuf->endIdx;
    cpSize = (right < size) ? right : size;

    ret = memcpy_s(ringbuf->fifo + ringbuf->endIdx, right, buf, cpSize);
    if (ret != OS_OK) {
        printf("write loop ring buffer failed, endIdx: %u, right:%u, cpSize=%u\n", ringbuf->endIdx, right, cpSize);
        return 0;
    }

    ringbuf->remain -= cpSize;
    ringbuf->endIdx += cpSize;

    if (ringbuf->endIdx == ringbuf->size) {
        ringbuf->endIdx = 0;
    }

    if (cpSize < size) {
        cpSize += OsRingbufWriteLinear(ringbuf, buf + cpSize, size - cpSize);
    }

    return cpSize;
}

U32 PRT_RingbufWrite(Ringbuf *ringbuf, const char *buf, U32 size)
{
    U32 cpSize;
    uintptr_t intSave;

    if ((ringbuf == NULL) || (buf == NULL) || (size == 0) || (ringbuf->fifo == NULL) ||
        (ringbuf->status != RBUF_INITED) || (ringbuf->remain == 0)) {
        return 0;
    }

    intSave = PRT_SplIrqLock(&ringbuf->lock);
    if (ringbuf->startIdx <= ringbuf->endIdx) {
        cpSize = OsRingbufWriteLoop(ringbuf, buf, size);
    } else {
        cpSize = OsRingbufWriteLinear(ringbuf, buf, size);
    }
    PRT_SplIrqUnlock(&ringbuf->lock, intSave);

    return cpSize;
}

static U32 OsRingbufReadLinear(Ringbuf *ringbuf, char *buf, U32 size)
{
    U32 ret;
    U32 cpSize;
    U32 remain;

    remain = ringbuf->endIdx - ringbuf->startIdx;
    cpSize = (remain < size) ? remain : size;

    if (cpSize == 0) {
        return 0;
    }

    ret = memcpy_s(buf, size, ringbuf->fifo + ringbuf->startIdx, cpSize);
    if (ret != OS_OK) {
        printf("read linear ring buffer failed, startIdx: %u, cpSize=%u\n", ringbuf->startIdx, cpSize);
        return 0;
    }

    ringbuf->remain += cpSize;
    ringbuf->startIdx += cpSize;

    return cpSize;
}

static U32 OsRingbufReadLoop(Ringbuf *ringbuf, char *buf, U32 size)
{
    U32 ret;
    U32 right;
    U32 cpSize;

    right = ringbuf->size - ringbuf->startIdx;
    cpSize = (right < size) ? right : size;

    ret = memcpy_s(buf, size, ringbuf->fifo + ringbuf->startIdx, cpSize);
    if (ret != OS_OK) {
        printf("read loop ring buffer failed, startIdx: %u, right:%u, cpSize=%u\n", ringbuf->startIdx, right, cpSize);
        return 0;
    }

    ringbuf->remain += cpSize;
    ringbuf->startIdx += cpSize;
    if (ringbuf->startIdx == ringbuf->size) {
        ringbuf->startIdx = 0;
    }

    if (cpSize < size) {
        cpSize += OsRingbufReadLinear(ringbuf, buf + cpSize, size - cpSize);
    }

    return cpSize;
}

U32 PRT_RingbufRead(Ringbuf *ringbuf, char *buf, U32 size)
{
    U32 cpSize;
    uintptr_t intSave;

    if ((ringbuf == NULL) || (buf == NULL) || (size == 0) || (ringbuf->fifo == NULL) ||
        (ringbuf->status != RBUF_INITED) || (ringbuf->remain == ringbuf->size)) {
        return 0;
    }

    intSave = PRT_SplIrqLock(&ringbuf->lock);
    if (ringbuf->startIdx >= ringbuf->endIdx) {
        cpSize = OsRingbufReadLoop(ringbuf, buf, size);
    } else {
        cpSize = OsRingbufReadLinear(ringbuf, buf, size);
    }
    PRT_SplIrqUnlock(&ringbuf->lock, intSave);

    return cpSize;
}

U32 PRT_RingbufInit(Ringbuf *ringbuf, char *fifo, U32 size)
{
    if ((ringbuf == NULL) || (fifo == NULL) || (ringbuf->status == RBUF_INITED) || (size == 0)) {
        return OS_ERROR;
    }

    (void)memset_s(ringbuf, sizeof(Ringbuf), 0, sizeof(Ringbuf));
    OsSpinLockInitInner(&ringbuf->lock.rawLock);
    ringbuf->size = size;
    ringbuf->remain = size;
    ringbuf->fifo = fifo;
    ringbuf->status = RBUF_INITED;

    return OS_OK;
}

void PRT_RingbufReset(Ringbuf *ringbuf)
{
    uintptr_t intSave;

    if ((ringbuf == NULL) || (ringbuf->status != RBUF_INITED)) {
        return;
    }

    intSave = PRT_SplIrqLock(&ringbuf->lock);
    ringbuf->startIdx = 0;
    ringbuf->endIdx = 0;
    ringbuf->remain = ringbuf->size;
    (void)memset_s(ringbuf->fifo, ringbuf->size, 0, ringbuf->size);
    PRT_SplIrqUnlock(&ringbuf->lock, intSave);

    return;
}