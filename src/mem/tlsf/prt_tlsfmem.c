/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-06-01
 * Description: TLSF 内存算法胶水层。
 *              不改动 TLSF 算法核心 prt_tlsf_core.c 的任何流程，仅做适配：
 *              1) 实现 TLSF 算法自己的初始化与释放接口 OsTlsfMemInit/OsTlsfMemFree，
 *                 以及算法无关的标准分配接口 OsMemAlloc/OsMemAllocAlign
 *                 （后两者被内核各模块直接调用，且填入 g_memArithAPI 分发表）。
 *              2) 提供 g_memArithAPI 分发表与 g_osMemAlloc 钩子指针的初始化。
 *              3) 维护 g_memTotalSize/g_memUsage/g_memPeakUsage/g_memStartAddr 统计量，
 *                 供 memInfo shell 命令使用（与 FSC 实现保持行为对齐）。
 *              4) 提供 litelibc 在 TLSF 模式下所需的 malloc_usable_size 实现。
 *              初始化由公共分派层 OsMemInit 按 CONFIG_OS_MEM_ARITH_TLSF 宏选用。
 *              锁由上层 prt_mem.c 的 PRT_HwiLock/PRT_HwiRestore 统一负责，胶水层不再加锁。
 */
#include "prt_typedef.h"
#include "prt_mem.h"
#include "../prt_mem_internal.h"
#include "prt_mem_external.h"
#include "prt_hook_external.h"
#include "prt_tlsfmem_external.h"
#include "prt_err_external.h"
#include "prt_cpu_external.h"
#include <stddef.h>
#include <stdint.h>
#include "prt_tlsf_core.h"

/* FSC 模式下由 prt_fscmem.c 定义的全局统计量与分发表，TLSF 模式下改由此处定义，
   保证 prt_mem.c / shell_memInfo.c 等引用方无需改动。 */
OS_SEC_BSS struct TagMemFuncLib g_memArithAPI;
uintptr_t g_memTotalSize = 0;
uintptr_t g_memUsage = 0;
uintptr_t g_memPeakUsage = 0;
uintptr_t g_memStartAddr = 0;

/* TLSF 内存池句柄（即 OsTlsfInit 传入的分区起始地址）。 */
static VOID *g_tlsfPool = NULL;

/* 节点头大小：算法核心中 OS_MEM_NODE_HEAD_SIZE = sizeof(struct OsMemUsedNodeHead)，
   而 OsMemUsedNodeHead 仅包裹 struct OsMemNodeHead，故二者大小相等。 */
#define OS_TLSF_NODE_HEAD_SIZE (sizeof(struct OsMemNodeHead))

/* 以下标志位宏与算法核心内部定义保持一致（核心文件中为私有宏，不在 prt_tlsf_core.h），
   胶水层复制一份用于统计/可用大小计算，不修改算法源码。 */
#define OS_TLSF_NODE_USED_FLAG        (1U << 31)
#define OS_TLSF_NODE_ALIGNED_FLAG     (1U << 30)
#define OS_TLSF_NODE_GET_SIZE(f)      ((f) & ~(OS_TLSF_NODE_USED_FLAG | OS_TLSF_NODE_ALIGNED_FLAG))
#define OS_TLSF_GAPSIZE_USED_FLAG     0x80000000U
#define OS_TLSF_GAPSIZE_ALIGNED_FLAG  0x40000000U
#define OS_TLSF_GET_ALIGNED_GAPSIZE(g)      ((g) & ~(OS_TLSF_GAPSIZE_USED_FLAG | OS_TLSF_GAPSIZE_ALIGNED_FLAG))
#define OS_TLSF_GET_GAPSIZE_ALIGNED_FLAG(g) ((g) & OS_TLSF_GAPSIZE_ALIGNED_FLAG)
#define OS_TLSF_GAPSIZE_CHECK(g)            (OS_TLSF_GET_GAPSIZE_ALIGNED_FLAG(g) && ((g) & OS_TLSF_GAPSIZE_USED_FLAG))

/* 根据用户指针还原节点头指针，逻辑等价于算法核心的 OsGetRealPtr。
   - 普通分配：用户指针紧邻节点头之后，ptr-4 落在 sizeAndFlag 字段，ALIGNED 标志为 0。
   - 对齐分配：用户指针前 4 字节为 gap 字，ALIGNED 标志为 1。 */
static struct OsMemNodeHead *OsTlsfNodeFromUser(void *ptr)
{
    UINT32 gapWord;
    UINTPTR realPtr;

    gapWord = *((UINT32 *)((UINTPTR)ptr - sizeof(UINT32)));
    if (OS_TLSF_GAPSIZE_CHECK(gapWord)) {
        return NULL;
    }
    realPtr = (UINTPTR)ptr;
    if (OS_TLSF_GET_GAPSIZE_ALIGNED_FLAG(gapWord)) {
        realPtr = (UINTPTR)ptr - (UINTPTR)OS_TLSF_GET_ALIGNED_GAPSIZE(gapWord);
    }
    return (struct OsMemNodeHead *)(realPtr - OS_TLSF_NODE_HEAD_SIZE);
}

static UINT32 OsTlsfNodeSize(void *ptr)
{
    struct OsMemNodeHead *node = OsTlsfNodeFromUser(ptr);
    if (node == NULL) {
        return 0;
    }
    return OS_TLSF_NODE_GET_SIZE(node->sizeAndFlag);
}

OS_SEC_TEXT U32 OsTlsfMemInit(uintptr_t addr, U32 size)
{
    UINT32 ret;
    if ((void *)(uintptr_t)addr == NULL) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_ISINVALID);
        return OS_ERRNO_MEM_INITADDR_ISINVALID;
    }
    if (size == 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITSIZE_INVALID);
        return OS_ERRNO_MEM_INITSIZE_INVALID;
    }
    /* 调用 TLSF 的初始化，流程不变。 */
    ret = OsTlsfInit((VOID *)(uintptr_t)addr, (UINT32)size);
    if (ret != OS_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_INVALID);
        return OS_ERRNO_MEM_INITADDR_INVALID;
    }
    g_tlsfPool = (VOID *)(uintptr_t)addr;
    g_memTotalSize = (uintptr_t)size;
    g_memStartAddr = addr;
    g_memUsage = 0;
    g_memPeakUsage = 0;

    /* 安装到 UniProton 的算法分发表与钩子指针。 */
    g_memArithAPI.alloc = OsMemAlloc;
    g_memArithAPI.allocAlign = OsMemAllocAlign;
    g_memArithAPI.free = OsTlsfMemFree;
    g_osMemAlloc = OsMemAlloc;
    return OS_OK;
}

OS_SEC_TEXT void *OsMemAlloc(enum MoudleId mid, U8 ptNo, U32 size)
{
    VOID *ptr;
    UINT32 nodeSize;

    (void)mid;
    (void)ptNo;
    /* 与 FSC 行为对齐：size==0 直接返回 NULL（对应 malloc_malloc_1_1 测试用例）。 */
    if (size == 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_SIZE_ZERO);
        return NULL;
    }
    ptr = OsTlsfAlloc(g_tlsfPool, (UINT32)size);
    if (ptr != NULL) {
        nodeSize = OsTlsfNodeSize(ptr);
        g_memUsage += nodeSize;
        if (g_memPeakUsage < g_memUsage) {
            g_memPeakUsage = g_memUsage;
        }
    }
    return ptr;
}

OS_SEC_TEXT void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow)
{
    VOID *ptr;
    UINT32 boundary;
    UINT32 nodeSize;

    (void)mid;
    (void)ptNo;
    if (alignPow >= MEM_ADDR_BUTT || alignPow < MEM_ADDR_ALIGN_004) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_ALIGNPOW_INVALID);
        return NULL;
    }
    if (size == 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_SIZE_ZERO);
        return NULL;
    }
    boundary = (UINT32)(1UL << (U32)alignPow);
    /* 与 FSC 的 OsFscMemAllocInner 对齐：boundary 小于指针宽度时钳到 sizeof(VOID*)，
       否则 OsTlsfAllocAlign 会因 OS_MEM_IS_ALIGNED(boundary,sizeof(VOID*)) 校验失败直接返回 NULL
       （UniProton 部分初始化代码会用 MEM_ADDR_ALIGN_004 即 boundary=4 调用）。 */
    if (boundary < sizeof(VOID *)) {
        boundary = sizeof(VOID *);
    }
    ptr = OsTlsfAllocAlign(g_tlsfPool, (UINT32)size, boundary);
    if (ptr != NULL) {
        nodeSize = OsTlsfNodeSize(ptr);
        g_memUsage += nodeSize;
        if (g_memPeakUsage < g_memUsage) {
            g_memPeakUsage = g_memUsage;
        }
    }
    return ptr;
}

OS_SEC_TEXT U32 OsTlsfMemFree(void *addr)
{
    UINT32 nodeSize;

    if (addr == NULL) {
        return OS_ERRNO_MEM_FREE_ADDR_INVALID;
    }
    /* 释放前先读取节点大小用于统计（释放后节点头失效）。 */
    nodeSize = OsTlsfNodeSize(addr);
    if (nodeSize == 0) {
        return OS_ERRNO_MEM_FREE_SH_DAMAGED;
    }
    if (OsTlsfFree(g_tlsfPool, (VOID *)addr) != OS_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_FREE_SH_DAMAGED);
        return OS_ERRNO_MEM_FREE_SH_DAMAGED;
    }
    if (g_memUsage >= (uintptr_t)nodeSize) {
        g_memUsage -= nodeSize;
    } else {
        g_memUsage = 0;
    }
    return OS_OK;
}

/*
 * litelibc 的 malloc_usable_size.c 在 TLSF 模式下被排除编译（见 src/libc/CMakeLists.txt），
 * 由本文件提供实现，供 realloc.c 调用。返回用户可用字节数 = 节点总大小 - 头 - 对齐gap。 */
size_t malloc_usable_size(void *p)
{
    struct OsMemNodeHead *node;
    UINT32 totalSize;
    UINT32 gapSize = 0;
    UINT32 gapWord;

    if (p == NULL) {
        return 0;
    }
    node = OsTlsfNodeFromUser(p);
    if (node == NULL) {
        return 0;
    }
    totalSize = OS_TLSF_NODE_GET_SIZE(node->sizeAndFlag);
    gapWord = *((UINT32 *)((UINTPTR)p - sizeof(UINT32)));
    if (OS_TLSF_GET_GAPSIZE_ALIGNED_FLAG(gapWord) && !OS_TLSF_GAPSIZE_CHECK(gapWord)) {
        gapSize = OS_TLSF_GET_ALIGNED_GAPSIZE(gapWord);
    }
    if (totalSize < (OS_TLSF_NODE_HEAD_SIZE + (UINT32)gapSize)) {
        return 0;
    }
    return (size_t)(totalSize - OS_TLSF_NODE_HEAD_SIZE - gapSize);
}
