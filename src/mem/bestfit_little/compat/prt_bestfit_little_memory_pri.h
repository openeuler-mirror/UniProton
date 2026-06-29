#ifndef PRT_BESTFIT_LITTLE_MEMORY_PRI_H
#define PRT_BESTFIT_LITTLE_MEMORY_PRI_H

#include "prt_bestfit_little_memory.h"
#include "prt_bestfit_little_memstat.h"
#include "prt_bestfit_little_slab_pri.h"
#include "prt_bestfit_little_spinlock.h"

#ifdef LOSCFG_KERNEL_MEM_BESTFIT_LITTLE
typedef struct LosHeapManager {
    struct LosHeapNode *head;
    struct LosHeapNode *tail;
    UINT32 size;
#ifdef LOSCFG_MEM_TASK_STAT
    Memstat stat;
#endif
#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    struct PrtBestfitLittleSlabControlHeader slabCtrlHdr;
#endif
} LosMemPoolInfo;
#endif

#define IS_ALIGNED(value, alignSize) ((((UINTPTR)(value)) & ((UINTPTR)((alignSize) - 1))) == 0)

extern SPIN_LOCK_S g_memSpin;
#define MEM_LOCK(state) LOS_SpinLockSave(&g_memSpin, &(state))
#define MEM_UNLOCK(state) LOS_SpinUnlockRestore(&g_memSpin, (state))

STATIC INLINE UINT32 OsMemMulPoolInit(VOID *pool, UINT32 size)
{
    return LOS_OK;
}

STATIC INLINE UINT32 OsMemMulPoolDeinit(const VOID *pool)
{
    return LOS_OK;
}

extern UINT32 OsMemSystemInit(UINTPTR memStart);
extern VOID OsMemInfoPrint(const VOID *pool);

#endif /* PRT_BESTFIT_LITTLE_MEMORY_PRI_H */
