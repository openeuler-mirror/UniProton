#ifndef PRT_BESTFIT_MEMSTAT_PRI_H
#define PRT_BESTFIT_MEMSTAT_PRI_H

#include "prt_bestfit_compat.h"

typedef struct {
    UINT32 memTotalUsed;
    UINT32 memTotalPeak;
} Memstat;

#define OS_MEM_ADD_USED(stat, usedSize, taskId)
#define OS_MEM_REDUCE_USED(stat, usedSize, taskId)
#define OS_MEM_CLEAR(taskId)
#define OS_MEM_USAGE(taskId) 0

#endif /* PRT_BESTFIT_MEMSTAT_PRI_H */
