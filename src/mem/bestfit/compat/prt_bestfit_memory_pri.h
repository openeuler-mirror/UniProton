#ifndef PRT_BESTFIT_MEMORY_PRI_H
#define PRT_BESTFIT_MEMORY_PRI_H

#include "prt_bestfit_memory.h"
#include "prt_bestfit_memstat_pri.h"
#include "prt_bestfit_slab_pri.h"
#include "prt_bestfit_spinlock.h"

typedef struct {
    VOID *pool;
    UINT32 poolSize;
#ifdef PRT_BESTFIT_MEM_TASK_STAT
    Memstat stat;
#endif
#ifdef PRT_BESTFIT_CFG_MEM_MUL_POOL
    VOID *nextPool;
#endif
#ifdef PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_EXTENTION
    struct PrtBestfitSlabControlHeader slabCtrlHdr;
#endif
} PrtBestfitMemPoolInfo;

#define IS_ALIGNED(value, alignSize) ((((UINTPTR)(value)) & ((UINTPTR)((alignSize) - 1))) == 0)

extern SPIN_LOCK_S g_memSpin;
#define MEM_LOCK(state) PRT_SpinLockSave(&g_memSpin, &(state))
#define MEM_UNLOCK(state) PRT_SpinUnlockRestore(&g_memSpin, (state))

STATIC INLINE UINT32 OsMemMulPoolInit(VOID *pool, UINT32 size)
{
    (void)pool;
    (void)size;
    return PRT_OK;
}

STATIC INLINE UINT32 OsMemMulPoolDeinit(const VOID *pool)
{
    (void)pool;
    return PRT_OK;
}

extern UINT32 OsMemSystemInit(UINTPTR memStart);
extern VOID OsMemInfoPrint(const VOID *pool);

#endif /* PRT_BESTFIT_MEMORY_PRI_H */
