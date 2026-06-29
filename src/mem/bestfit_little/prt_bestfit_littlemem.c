/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit_little allocator glue for UniProton memory framework.
 */
#include "prt_typedef.h"
#include "prt_mem.h"
#include "../prt_mem_internal.h"
#include "prt_bestfit_littlemem_external.h"
#include "prt_err_external.h"
#include "prt_hook_external.h"
#include "prt_bestfit_little_memory.h"
#include "prt_bestfit_little_memory_internal.h"
#include "prt_bestfit_little_slab_pri.h"
#include <stddef.h>

#ifdef OsMemAlloc
#undef OsMemAlloc
#endif

#ifdef OsMemFree
#undef OsMemFree
#endif

OS_SEC_BSS struct TagMemFuncLib g_memArithAPI;
uintptr_t g_memTotalSize = 0;
uintptr_t g_memUsage = 0;
uintptr_t g_memPeakUsage = 0;
uintptr_t g_memStartAddr = 0;

static VOID *g_bestfitLittlePool = NULL;

static UINT32 OsBestfitLittleUsageGet(VOID)
{
    UINT32 used = PRT_BestfitLittleMemTotalUsedGet(g_bestfitLittlePool);

    return (used == OS_NULL_INT) ? 0 : used;
}

static VOID OsBestfitLittleUsageUpdate(VOID)
{
    g_memUsage = (uintptr_t)OsBestfitLittleUsageGet();
    if (g_memPeakUsage < g_memUsage) {
        g_memPeakUsage = g_memUsage;
    }
}

OS_SEC_TEXT U32 OsBestfitLittleMemInit(uintptr_t addr, U32 size)
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
    if (PRT_BestfitLittleMemInit((VOID *)(uintptr_t)addr, size) != LOS_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_INITADDR_INVALID);
        return OS_ERRNO_MEM_INITADDR_INVALID;
    }

    g_bestfitLittlePool = (VOID *)(uintptr_t)addr;
    g_memTotalSize = (uintptr_t)size;
    g_memStartAddr = addr;
    g_memUsage = (uintptr_t)OsBestfitLittleUsageGet();
    g_memPeakUsage = g_memUsage;

    g_memArithAPI.alloc = OsMemAlloc;
    g_memArithAPI.allocAlign = OsMemAllocAlign;
    g_memArithAPI.free = OsBestfitLittleMemFree;
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
    ptr = PRT_BestfitLittleMemAlloc(g_bestfitLittlePool, size);
    if (ptr != NULL) {
        OsBestfitLittleUsageUpdate();
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
    ptr = PRT_BestfitLittleMemAllocAlign(g_bestfitLittlePool, size, boundary);
    if (ptr != NULL) {
        OsBestfitLittleUsageUpdate();
    }
    return ptr;
}

OS_SEC_TEXT U32 OsBestfitLittleMemFree(void *addr)
{
    if (addr == NULL) {
        return OS_ERRNO_MEM_FREE_ADDR_INVALID;
    }
    if (PRT_BestfitLittleMemFree(g_bestfitLittlePool, addr) != LOS_OK) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_FREE_SH_DAMAGED);
        return OS_ERRNO_MEM_FREE_SH_DAMAGED;
    }
    g_memUsage = (uintptr_t)OsBestfitLittleUsageGet();
    return OS_OK;
}

size_t malloc_usable_size(void *p)
{
    UINT32 gapSize;
    VOID *ptr = p;
    struct LosHeapNode *node;

    if (p == NULL) {
        return 0;
    }

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    UINT32 slabSize = OsSlabMemCheck(g_bestfitLittlePool, p);
    if (slabSize != (UINT32)-1) {
        return (size_t)slabSize;
    }
#endif

    gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(UINTPTR)));
    if (OS_MEM_GET_ALIGN_FLAG(gapSize)) {
        ptr = (VOID *)((UINTPTR)ptr - OS_MEM_GET_ALIGN_GAPSIZE(gapSize));
    }
    node = ((struct LosHeapNode *)ptr) - 1;
    if (!node->used) {
        return 0;
    }
    return (size_t)node->size;
}
