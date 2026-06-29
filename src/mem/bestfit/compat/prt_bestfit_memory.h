#ifndef PRT_BESTFIT_MEMORY_H
#define PRT_BESTFIT_MEMORY_H

#include "prt_bestfit_compat.h"

typedef struct {
    UINT32 uwTotalUsedSize;
    UINT32 uwTotalFreeSize;
    UINT32 uwMaxFreeNodeSize;
    UINT32 uwUsedNodeNum;
    UINT32 uwFreeNodeNum;
#ifdef PRT_BESTFIT_MEM_TASK_STAT
    UINT32 uwUsageWaterLine;
#endif
} PRT_BESTFIT_MEM_POOL_STATUS;

extern UINT32 PRT_BestfitMemInit(VOID *pool, UINT32 size);
extern VOID *PRT_BestfitMemAlloc(VOID *pool, UINT32 size);
extern VOID *PRT_BestfitMemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);
extern VOID *PRT_BestfitMemRealloc(VOID *pool, VOID *ptr, UINT32 size);
extern UINT32 PRT_BestfitMemFree(VOID *pool, VOID *ptr);
extern UINT32 PRT_BestfitMemInfoGet(VOID *pool, PRT_BESTFIT_MEM_POOL_STATUS *poolStatus);
extern UINT32 PRT_BestfitMemTotalUsedGet(VOID *pool);
extern UINT32 PRT_BestfitMemPoolSizeGet(const VOID *pool);
extern UINT32 PRT_BestfitMemIntegrityCheck(VOID *pool);

#endif /* PRT_BESTFIT_MEMORY_H */
