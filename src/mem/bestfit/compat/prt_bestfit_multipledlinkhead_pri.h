#ifndef PRT_BESTFIT_MULTIPLE_DLINK_HEAD_PRI_H
#define PRT_BESTFIT_MULTIPLE_DLINK_HEAD_PRI_H

#include "prt_bestfit_list.h"

#define OS_MAX_MULTI_DLNK_LOG2  29
#define OS_MIN_MULTI_DLNK_LOG2  4
#define OS_MULTI_DLNK_NUM       ((OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2) + 1)
#define OS_DLNK_HEAD_SIZE       OS_MULTI_DLNK_HEAD_SIZE
#define OS_MULTI_DLNK_HEAD_SIZE sizeof(PrtBestfitMultipleDlinkHead)

typedef struct {
    PRT_DL_LIST listHead[OS_MULTI_DLNK_NUM];
} PrtBestfitMultipleDlinkHead;

STATIC INLINE PRT_DL_LIST *OsDLnkNextMultiHead(VOID *headAddr, PRT_DL_LIST *listNodeHead)
{
    PrtBestfitMultipleDlinkHead *head = (PrtBestfitMultipleDlinkHead *)headAddr;

    return (&head->listHead[OS_MULTI_DLNK_NUM - 1] == listNodeHead) ? NULL : (listNodeHead + 1);
}

extern VOID OsDLnkInitMultiHead(VOID *headAddr);
extern PRT_DL_LIST *OsDLnkMultiHead(VOID *headAddr, UINT32 size);

#endif /* PRT_BESTFIT_MULTIPLE_DLINK_HEAD_PRI_H */
