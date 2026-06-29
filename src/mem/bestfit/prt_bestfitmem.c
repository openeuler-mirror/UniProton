/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit allocator glue for UniProton memory framework.
 */
#include "prt_typedef.h"
#include "prt_mem.h"
#include "../prt_mem_internal.h"
#include "prt_bestfitmem_external.h"
#include "prt_err_external.h"
#include "prt_hook_external.h"
#include "prt_bestfit_memory.h"
#include "prt_bestfit_memory_internal.h"
#include "prt_bestfit_slab_pri.h"
#include <stddef.h>

OS_SEC_BSS struct TagMemFuncLib g_memArithAPI;
uintptr_t g_memTotalSize = 0;
uintptr_t g_memUsage = 0;
uintptr_t g_memPeakUsage = 0;
uintptr_t g_memStartAddr = 0;

static VOID *g_bestfitPool = NULL;

static UINT32 OsBestfitUsageGet(VOID)
{
    UINT32 used = PRT_BestfitMemTotalUsedGet(g_bestfitPool);

    return (used == PRT_NOK) ? 0 : used;
}

static VOID OsBestfitUsageUpdate(VOID)
{
    g_memUsage = (uintptr_t)OsBestfitUsageGet();
    if (g_memPeakUsage < g_memUsage) {
        g_memPeakUsage = g_memUsage;
    }
}

OS_SEC_TEXT U32 OsBestfitMemInit(uintptr_t addr, U32 size)
{
    if ((VOID *)(uintptr_t)addr == NULL) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_ISINVALID);
        return OS_ERRNO_MEM_INITADDR_ISINVALID;
    }
    if ((addr & (uintptr_t)(sizeof(uintptr_t) - 1)) != 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_INVALID);
        return OS_ERRNO_MEM_INITADDR_INVALID;
    }
    if ((size & (U32)(sizeof(uintptr_t) - 1)) != 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITSIZE_INVALID);
        return OS_ERRNO_MEM_INITSIZE_INVALID;
    }
    if (PRT_BestfitMemInit((VOID *)(uintptr_t)addr, size) != PRT_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_INVALID);
        return OS_ERRNO_MEM_INITADDR_INVALID;
    }

    g_bestfitPool = (VOID *)(uintptr_t)addr;
    g_memTotalSize = (uintptr_t)size;
    g_memStartAddr = addr;
    g_memUsage = (uintptr_t)OsBestfitUsageGet();
    g_memPeakUsage = g_memUsage;

    g_memArithAPI.alloc = OsMemAlloc;
    g_memArithAPI.allocAlign = OsMemAllocAlign;
    g_memArithAPI.free = OsBestfitMemFree;
    g_osMemAlloc = OsMemAlloc;
    return OS_OK;
}

OS_SEC_TEXT void *OsMemAlloc(enum MoudleId mid, U8 ptNo, U32 size)
{
    VOID *ptr;

    (void)mid;
    (void)ptNo;
    if (size == 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_SIZE_ZERO);
        return NULL;
    }
    ptr = PRT_BestfitMemAlloc(g_bestfitPool, size);
    if (ptr != NULL) {
        OsBestfitUsageUpdate();
    }
    return ptr;
}

OS_SEC_TEXT void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow)
{
    UINT32 boundary;
    VOID *ptr;

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
    if (boundary < sizeof(VOID *)) {
        boundary = sizeof(VOID *);
    }
    ptr = PRT_BestfitMemAllocAlign(g_bestfitPool, size, boundary);
    if (ptr != NULL) {
        OsBestfitUsageUpdate();
    }
    return ptr;
}

OS_SEC_TEXT U32 OsBestfitMemFree(void *addr)
{
    if (addr == NULL) {
        return OS_ERRNO_MEM_FREE_ADDR_INVALID;
    }
    if (PRT_BestfitMemFree(g_bestfitPool, addr) != PRT_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_FREE_SH_DAMAGED);
        return OS_ERRNO_MEM_FREE_SH_DAMAGED;
    }
    g_memUsage = (uintptr_t)OsBestfitUsageGet();
    return OS_OK;
}

size_t malloc_usable_size(void *p)
{
    UINT32 gapSize;
    VOID *ptr = p;
    PrtBestfitMemDynNode *node;
    UINT32 nodeSize;
#ifdef PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_EXTENTION
    UINT32 slabSize;
#endif

    if (p == NULL) {
        return 0;
    }

#ifdef PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_EXTENTION
    slabSize = OsSlabMemCheck(g_bestfitPool, p);
    if (slabSize != (UINT32)-1) {
        return (size_t)slabSize;
    }
#endif

    gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(UINT32)));
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize)) {
        if (OS_MEM_NODE_GET_USED_FLAG(gapSize)) {
            return 0;
        }
        ptr = (VOID *)((UINTPTR)ptr - OS_MEM_NODE_GET_ALIGNED_GAPSIZE(gapSize));
    }

    node = (PrtBestfitMemDynNode *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        return 0;
    }

    nodeSize = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
    if (nodeSize <= OS_MEM_NODE_HEAD_SIZE) {
        return 0;
    }
    return (size_t)(nodeSize - OS_MEM_NODE_HEAD_SIZE - ((UINTPTR)p - (UINTPTR)ptr));
}
