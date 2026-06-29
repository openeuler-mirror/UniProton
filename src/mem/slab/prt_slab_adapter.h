#ifndef PRT_SLAB_ADAPTER_H
#define PRT_SLAB_ADAPTER_H

#include "prt_buildef.h"

#if defined(OS_MEM_ARITH_BESTFIT_LITTLE)
#include "prt_bestfit_little_slab_pri.h"
#include "prt_bestfit_little_memory_pri.h"

#define PrtSlabControlHeader struct PrtBestfitLittleSlabControlHeader
#define PrtSlabStatus PrtBestfitLittleSlabStatus
#define PRT_SLAB_OK LOS_OK
#define PRT_SLAB_NOK LOS_NOK
#define PRT_SLAB_SIZE_CFG PRT_BestfitLittleSlabSizeCfg

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#define PRT_SLAB_AUTO_EXPANSION_MODE
#endif

STATIC INLINE VOID *PrtSlabHeapAlloc(VOID *pool, UINT32 size)
{
    return OsPrtHeapAllocForSlab(pool, size);
}

STATIC INLINE UINT32 PrtSlabHeapFree(VOID *pool, VOID *mem)
{
    return OsPrtHeapFreeForSlab(pool, mem);
}

STATIC INLINE VOID *PrtSlabCtrlHdrGet(const VOID *pool)
{
    return &((LosMemPoolInfo *)pool)->slabCtrlHdr;
}

#elif defined(OS_MEM_ARITH_BESTFIT)
#include "prt_bestfit_slab_pri.h"
#include "prt_bestfit_memory_pri.h"

#define PrtSlabControlHeader struct PrtBestfitSlabControlHeader
#define PrtSlabStatus PrtBestfitSlabStatus
#define PRT_SLAB_OK PRT_OK
#define PRT_SLAB_NOK PRT_NOK
#define PRT_SLAB_SIZE_CFG PRT_BestfitSlabSizeCfg

#ifdef PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#define PRT_SLAB_AUTO_EXPANSION_MODE
#endif

STATIC INLINE VOID *PrtSlabHeapAlloc(VOID *pool, UINT32 size)
{
    return OsBestfitHeapAlloc(pool, size);
}

STATIC INLINE UINT32 PrtSlabHeapFree(VOID *pool, VOID *mem)
{
    return OsBestfitHeapFree(pool, mem);
}

STATIC INLINE VOID *PrtSlabCtrlHdrGet(const VOID *pool)
{
    return &((PrtBestfitMemPoolInfo *)pool)->slabCtrlHdr;
}

#else
#error "slab common source must be built with a bestfit allocator"
#endif

#endif /* PRT_SLAB_ADAPTER_H */
