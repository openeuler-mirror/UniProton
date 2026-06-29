#ifndef PRT_BESTFIT_LITTLE_MEMORY_H
#define PRT_BESTFIT_LITTLE_MEMORY_H

#include "prt_bestfit_little_config.h"
#include "prt_bestfit_little_base.h"

typedef VOID (*MALLOC_HOOK)(VOID);

extern MALLOC_HOOK g_MALLOC_HOOK;
extern UINT8 *m_aucSysMem0;
extern UINT8 *m_aucSysMem1;
extern UINTPTR g_sys_mem_addr_end;
extern UINTPTR g_excInteractMemSize;

typedef struct {
    UINT32 uwTotalUsedSize;
    UINT32 uwTotalFreeSize;
    UINT32 uwMaxFreeNodeSize;
    UINT32 uwUsedNodeNum;
    UINT32 uwFreeNodeNum;
    UINT32 uwUsageWaterLine;
} PRT_BESTFIT_LITTLE_MEM_POOL_STATUS;

extern UINT32 PRT_BestfitLittleMemInit(VOID *pool, UINT32 size);
extern VOID *PRT_BestfitLittleMemAlloc(VOID *pool, UINT32 size);
extern VOID *PRT_BestfitLittleMemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);
extern VOID *PRT_BestfitLittleMemRealloc(VOID *pool, VOID *ptr, UINT32 size);
extern UINT32 PRT_BestfitLittleMemFree(VOID *pool, VOID *ptr);
extern UINT32 PRT_BestfitLittleMemInfoGet(VOID *pool, PRT_BESTFIT_LITTLE_MEM_POOL_STATUS *status);
extern UINT32 PRT_BestfitLittleMemTotalUsedGet(VOID *pool);
extern UINT32 PRT_BestfitLittleMemPoolSizeGet(const VOID *pool);
extern UINT32 PRT_BestfitLittleMemIntegrityCheck(VOID *pool);

#endif /* PRT_BESTFIT_LITTLE_MEMORY_H */
