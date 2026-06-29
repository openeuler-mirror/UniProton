#ifndef PRT_BESTFIT_LITTLE_SLAB_PRI_H
#define PRT_BESTFIT_LITTLE_SLAB_PRI_H

#include "prt_bestfit_little_slab.h"
#include "prt_bestfit_little_base.h"

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION

#define SLAB_MEM_COUNT 4
#define SLAB_MEM_CALSS_STEP_SIZE 0x10U
#define SLAB_MEM_MAX_SIZE (SLAB_MEM_CALSS_STEP_SIZE << (SLAB_MEM_COUNT - 1))

typedef struct tagPrtBestfitLittleSlabStatus {
    UINT32 totalSize;
    UINT32 usedSize;
    UINT32 freeSize;
    UINT32 allocCount;
    UINT32 freeCount;
} PrtBestfitLittleSlabStatus;

typedef struct tagOsSlabBlockNode {
    UINT16 magic;
    UINT8 blkSz;
    UINT8 recordId;
    UINT32 reserved;
} OsSlabBlockNode;

struct AtomicBitset {
    UINT32 numBits;
    UINT32 words[0];
};

typedef struct tagOsSlabAllocator {
    UINT32 itemSz;
    UINT8 *dataChunks;
    struct AtomicBitset *bitset;
} OsSlabAllocator;

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
typedef struct tagOsSlabMemAllocator {
    struct tagOsSlabMemAllocator *next;
    OsSlabAllocator *slabAlloc;
} OsSlabMemAllocator;
#endif

typedef struct tagOsSlabMem {
    UINT32 blkSz;
    UINT32 blkCnt;
    UINT32 blkUsedCnt;
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    UINT32 allocatorCnt;
    OsSlabMemAllocator *bucket;
#else
    OsSlabAllocator *alloc;
#endif
} OsSlabMem;

struct PrtBestfitLittleSlabControlHeader {
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocator *allocatorBucket;
#endif
    OsSlabMem slabClass[SLAB_MEM_COUNT];
};

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#define SLAB_MEM_DFEAULT_BUCKET_CNT 1
#endif

#define LOW_BITS_MASK 31U
#define OS_SLAB_MAGIC 0xdede
#define OS_SLAB_BLOCK_HEAD_GET(ptr) ((OsSlabBlockNode *)(VOID *)((UINT8 *)(ptr) - sizeof(OsSlabBlockNode)))
#define OS_SLAB_BLOCK_MAGIC_SET(slabNode) (((OsSlabBlockNode *)(slabNode))->magic = (UINT16)OS_SLAB_MAGIC)
#define OS_SLAB_BLOCK_MAGIC_GET(slabNode) (((OsSlabBlockNode *)(slabNode))->magic)
#define OS_SLAB_BLOCK_SIZE_SET(slabNode, size) (((OsSlabBlockNode *)(slabNode))->blkSz = (UINT8)(size))
#define OS_SLAB_BLOCK_SIZE_GET(slabNode) (((OsSlabBlockNode *)(slabNode))->blkSz)
#define OS_SLAB_BLOCK_ID_SET(slabNode, id) (((OsSlabBlockNode *)(slabNode))->recordId = (id))
#define OS_SLAB_BLOCK_ID_GET(slabNode) (((OsSlabBlockNode *)(slabNode))->recordId)
#define OS_ALLOC_FROM_SLAB_CHECK(slabNode) (((OsSlabBlockNode *)(slabNode))->magic == (UINT16)OS_SLAB_MAGIC)
#define ATOMIC_BITSET_SZ(numbits) (sizeof(struct AtomicBitset) + ((numbits) + LOW_BITS_MASK) / 8)
#define OS_SLAB_LOG2(value) ((UINT32)(32 - CLZ(value) - 1))
#define OS_SLAB_CLASS_LEVEL_GET(size) \
    (OS_SLAB_LOG2((size - 1) >> (OS_SLAB_LOG2(SLAB_MEM_CALSS_STEP_SIZE - 1))))

extern OsSlabAllocator *OsSlabAllocatorNew(VOID *pool, UINT32 itemSz, UINT32 itemAlign, UINT32 numItems);
extern VOID OsSlabAllocatorDestroy(VOID *pool, OsSlabAllocator *allocator);
extern VOID *OsSlabAllocatorAlloc(OsSlabAllocator *allocator);
extern BOOL OsSlabAllocatorFree(OsSlabAllocator *allocator, VOID *ptr);
extern BOOL OsSlabAllocatorEmpty(const OsSlabAllocator *allocator);
extern VOID OsSlabAllocatorGetSlabInfo(const OsSlabAllocator *allocator, UINT32 *itemSize,
    UINT32 *itemCnt, UINT32 *curUsage);
extern BOOL OsSlabAllocatorCheck(const OsSlabAllocator *allocator, VOID *ptr);
extern VOID OsSlabMemInit(VOID *pool, UINT32 size);
extern VOID OsSlabMemDeinit(VOID *pool);
extern VOID *OsSlabMemAlloc(VOID *pool, UINT32 sz);
extern BOOL OsSlabMemFree(VOID *pool, VOID *ptr);
extern UINT32 OsSlabMemCheck(const VOID *pool, VOID *ptr);
extern UINT32 OsSlabStatisticsGet(const VOID *pool, PrtBestfitLittleSlabStatus *status);
extern UINT32 OsSlabGetMaxFreeBlkSize(const VOID *pool);
extern VOID *OsSlabCtrlHdrGet(const VOID *pool);

#else

STATIC INLINE VOID OsSlabMemInit(VOID *pool, UINT32 size)
{
}

STATIC INLINE VOID OsSlabMemDeinit(VOID *pool)
{
}

STATIC INLINE VOID *OsSlabMemAlloc(VOID *pool, UINT32 size)
{
    return NULL;
}

STATIC INLINE BOOL OsSlabMemFree(VOID *pool, VOID *ptr)
{
    return FALSE;
}

STATIC INLINE UINT32 OsSlabMemCheck(const VOID *pool, VOID *ptr)
{
    return (UINT32)-1;
}

#endif /* LOSCFG_KERNEL_MEM_SLAB_EXTENTION */

#endif /* PRT_BESTFIT_LITTLE_SLAB_PRI_H */
