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
 * Create: 2024-03-19
 * Description: 混合部署日志功能。
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "prt_typedef.h"
#include "prt_attr_external.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "prt_atomic.h"
#include "prt_log_internal.h"
#include "securec.h"

#define LOG_FACILITY_NUM 8

static volatile uintptr_t g_logMemBase = 0;
static volatile U32 g_sequenceNum = 0;
static volatile U8 g_logOn = 1;

static volatile enum OsLogLevel g_logFilter[LOG_FACILITY_NUM] =
    {OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE, OS_LOG_NONE};

static OS_SEC_L2_TEXT U32 OsLog(enum OsLogLevel level, enum OsLogFacility facility, const char *str, size_t strLen);

#if defined(OS_OPTION_SMP)
static struct PrtSpinLock g_logLock;
static OS_SEC_L2_TEXT uintptr_t OsLogLockOn()
{
    return PRT_SplIrqLock(&g_logLock);
}

static OS_SEC_L2_TEXT void OsLogLockRestore(uintptr_t value)
{
    PRT_SplIrqUnlock(&g_logLock, value);
}
#else
static OS_SEC_L2_TEXT uintptr_t OsLogLockOn()
{
    return PRT_HwiLock();
}

static OS_SEC_L2_TEXT void OsLogLockRestore(uintptr_t value)
{
    return PRT_HwiRestore(value);
}
#endif

OS_SEC_ALW_INLINE INLINE U32 OsAtomicReadU32(volatile void *addr)
{
    return *(volatile U32 *)addr;
}

OS_SEC_ALW_INLINE INLINE void OsAtomicSetU32(volatile void *addr, U32 val)
{
    *(volatile U32 *)addr = val;
}

OS_SEC_ALW_INLINE INLINE U32 OsCheckLog(enum OsLogLevel level, enum OsLogFacility facility)
{
    if (level < OS_LOG_EMERG || level > OS_LOG_DEBUG || facility < OS_LOG_F0 || facility > OS_LOG_F7) {
        return -1;
    }
    return 0;
}

/* 只能调用一次 */
OS_SEC_L2_TEXT U32 PRT_LogInit(uintptr_t memBase)
{
    int ret;
    LOAD_FENCE();
    if (g_logMemBase != 0) {
        return 0;
    }
#if defined(OS_OPTION_SMP)
    ret = PRT_SplLockInit(&g_logLock);
    if (ret) {
        return -1;
    }
#endif
    g_logMemBase = memBase;
    STORE_FENCE();
    return 0;
}

/* 从核可能需要通过该函数判断 */
OS_SEC_L2_TEXT bool PRT_IsLogInit()
{
    LOAD_FENCE();
    if (g_logMemBase != 0) {
        return true;
    }
    return false;
}

/* 原子操作，不需要锁 */
OS_SEC_L2_TEXT void PRT_LogOn()
{
    g_logOn = 1;
}

/* 原子操作，不需要锁 */
OS_SEC_L2_TEXT void PRT_LogOff()
{
    g_logOn = 0;
}

/* 根据facility过滤, 优先级比loglevel低的会被过滤, emerge无法被过滤 */
/* 原子操作, 不需要锁 */
OS_SEC_L2_TEXT U32 PRT_LogSetFilterByFacility(enum OsLogFacility facility, enum OsLogLevel level)
{
    if (level < OS_LOG_EMERG || level > OS_LOG_NONE || facility < OS_LOG_F0 || facility > OS_LOG_F7) {
        return -1;
    }
    g_logFilter[facility - OS_LOG_F0] = level;
    return 0;
}

/* 对每个Facility都用相同的优先级过滤 */
/* 并发使用的情况下可能导致结果不符合预期，但影响不大，可以忽略 */
OS_SEC_L2_TEXT U32 PRT_LogSetFilter(enum OsLogLevel level)
{
    int i;
    if (level < OS_LOG_EMERG || level > OS_LOG_NONE) {
        return -1;
    }
    for (i = 0; i < LOG_FACILITY_NUM; i++) {
        g_logFilter[i] = level;
    }
    return 0;
}

static OS_SEC_L2_TEXT void OsLogGetTailAndHead(U32 *head, U32 *tail)
{
    /* 保证获取head值时, tail未发生变化 (U32翻转并且刚好回到原点概率过小）*/
    U32 tail1, tail2, currHead;
    volatile U32 *const headPtr = (U32 *)(g_logMemBase + HEAD_PTR_OFFSET);
    volatile U32 *const tailPtr = (U32 *)(g_logMemBase + TAIL_PTR_OFFSET);
    do {
        tail1 = OsAtomicReadU32(tailPtr);
        LOAD_FENCE();
        currHead = OsAtomicReadU32(headPtr);
        LOAD_FENCE();
        tail2 = OsAtomicReadU32(tailPtr);
    } while (tail1 != tail2);
    *tail = tail2;
    *head = currHead;
    return;
}

static OS_SEC_L2_TEXT U32 OsLogUpdateTail(U32 *tail)
{
    uintptr_t intSave;
    U32 currHead, currTail;
    bool isOverflow;
    volatile U32 *const tailPtr = (U32 *)(g_logMemBase + TAIL_PTR_OFFSET);

    /* 获取 tail, head, 检查范围, 并更新tail */
    while (1) {
        isOverflow = false;
        OsLogGetTailAndHead(&currHead, &currTail);
        if (currHead >  U32_MAX - BUFFER_BLOCK_NUM) {
            isOverflow = true;
            currTail += 2 * BUFFER_BLOCK_NUM;
            currHead += 2 * BUFFER_BLOCK_NUM;
        }
        if (currTail < currHead || currTail >= currHead + BUFFER_BLOCK_NUM) {
            /* 范围错误 */
            return -1;
        }
        if (isOverflow) {
            currTail -= 2 * BUFFER_BLOCK_NUM;
        }
        intSave = OsLogLockOn();
        M_FENCE()
        /* 检查期间, tail未发生变化, 多核可以用原子操作cmpxchg替换 */
        if (currTail == OsAtomicReadU32(tailPtr)) {
            OsAtomicSetU32(tailPtr, currTail + 1);
            OsLogLockRestore(intSave);
            *tail = currTail;
            return 0;
        }
        OsLogLockRestore(intSave);
    }
}

static OS_SEC_L2_TEXT U32 OsLog(enum OsLogLevel level, enum OsLogFacility facility, const char *str, size_t strLen)
{
    uintptr_t intSave;
    U32 currTail, logIndex, sequenceNum, ret;
    U8 validFlag;
    U8 *targetMem;
    struct logHeader header;
    TskHandle taskPid = -1;
    struct timespec ts = {0, 0};
    volatile U8 *const validPtr = (U8 *)(g_logMemBase + VALID_FLAGS_OFFSET);

    intSave = OsLogLockOn();
    /* 多核可以用原子自增替换 */
    sequenceNum = g_sequenceNum;
    g_sequenceNum++;
    OsLogLockRestore(intSave);

    if (OsLogUpdateTail(&currTail)) {
        return -1;
    }

    /* 计算当前的有效标记位, 奇数圈1, 偶数圈0 */
    logIndex = currTail % (2 * BUFFER_BLOCK_NUM);
    if (logIndex < BUFFER_BLOCK_NUM) {
        validFlag = 1;
    } else {
        validFlag = 0;
    }
    logIndex = logIndex % (BUFFER_BLOCK_NUM);

    clock_gettime(CLOCK_REALTIME, &ts);
    header.sec = ts.tv_sec;
    header.nanoSec = ts.tv_nsec;
    header.sequenceNum = sequenceNum;
    PRT_TaskSelf(&taskPid);
    header.taskPid = taskPid;
    header.len = strLen;
    header.facility = facility;
    header.level = level;

    targetMem = (U8 *)g_logMemBase + (logIndex * BUFFER_BLOCK_SIZE);
    (void)memcpy_s(targetMem, sizeof(struct logHeader), &header, sizeof(struct logHeader));
    ret = memcpy_s(targetMem + sizeof(struct logHeader), LOG_MAX_SIZE, str, strLen);

    STORE_FENCE();

    /* 设置有效标记位 */
    *(volatile U8 *)(validPtr + logIndex) = validFlag;
    if (ret != EOK) {
        return -1;
    }
    /* 保证有效标记位及时更新，不然最后一个日志可能获取较慢 */
    STORE_FENCE();
    return 0;
}

OS_SEC_L2_TEXT U32 PRT_Log(enum OsLogLevel level, enum OsLogFacility facility, const char *str, size_t strLen)
{
    LOAD_FENCE();
    if (g_logMemBase == 0) {
        return -1;
    }
    if (OsCheckLog(level, facility) || str == NULL || strLen == 0) {
        return -1;
    }
    /* 检查是否被过滤 */
    if ((!g_logOn) || (level >= g_logFilter[facility - OS_LOG_F0])) {
        return 0;
    }

    if (strLen > (LOG_MAX_SIZE - 1)) {
        strLen = LOG_MAX_SIZE - 1;
    }
    return OsLog(level, facility, str, strLen);
}

extern U32 PRT_LogFormat(enum OsLogLevel level, enum OsLogFacility facility, const char *fmt, ...)
{
    int len;
    va_list vaList;
    char buff[BUFFER_BLOCK_SIZE];

    LOAD_FENCE();
    if (g_logMemBase == 0) {
        return -1;
    }
    if (OsCheckLog(level, facility) || fmt == NULL) {
        return -1;
    }

    /* 检查是否被过滤 */
    if ((!g_logOn) || (level >= g_logFilter[facility - OS_LOG_F0])) {
        return 0;
    }

    memset_s(buff, BUFFER_BLOCK_SIZE, 0, BUFFER_BLOCK_SIZE);
    va_start(vaList, fmt);
    // 字符串格式化由用户负责
    len = vsnprintf_s(buff, BUFFER_BLOCK_SIZE, BUFFER_BLOCK_SIZE, fmt, vaList);
    va_end(vaList);
    if (len < 0) {
        return len;
    }

    if (len > (LOG_MAX_SIZE - 1)) {
        len = LOG_MAX_SIZE - 1;
    }
    return OsLog(level, facility, buff, len);
}
