#ifndef PRT_BESTFIT_LMS_PRI_H
#define PRT_BESTFIT_LMS_PRI_H

#include "prt_bestfit_compat.h"

STATIC INLINE UINT32 OsLmsMemInit(const VOID *pool, UINT32 size)
{
    (void)pool;
    return size;
}

STATIC INLINE VOID OsLmsSetAfterMalloc(const VOID *ptr)
{
    (void)ptr;
}

STATIC INLINE VOID OsLmsSetAfterFree(const VOID *ptr)
{
    (void)ptr;
}

#endif /* PRT_BESTFIT_LMS_PRI_H */
