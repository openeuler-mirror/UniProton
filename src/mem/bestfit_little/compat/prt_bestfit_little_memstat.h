#ifndef PRT_BESTFIT_LITTLE_MEMSTAT_H
#define PRT_BESTFIT_LITTLE_MEMSTAT_H

#include "prt_bestfit_little_compat.h"

#define TASK_NUM (LOSCFG_BASE_CORE_TSK_LIMIT + 1)

#ifdef LOSCFG_MEM_TASK_STAT
typedef struct {
    UINT32 memUsed;
    UINT32 memPeak;
} TaskMemUsedInfo;

typedef struct {
    UINT32 memTotalUsed;
    UINT32 memTotalPeak;
    TaskMemUsedInfo taskMemstats[TASK_NUM];
} Memstat;

STATIC INLINE VOID OsMemstatTaskUsedInc(Memstat *stat, UINT32 usedSize, UINT32 taskId)
{
    if ((stat == NULL) || (taskId >= TASK_NUM)) {
        return;
    }
    stat->memTotalUsed += usedSize;
    if (stat->memTotalPeak < stat->memTotalUsed) {
        stat->memTotalPeak = stat->memTotalUsed;
    }
    stat->taskMemstats[taskId].memUsed += usedSize;
    if (stat->taskMemstats[taskId].memPeak < stat->taskMemstats[taskId].memUsed) {
        stat->taskMemstats[taskId].memPeak = stat->taskMemstats[taskId].memUsed;
    }
}

STATIC INLINE VOID OsMemstatTaskUsedDec(Memstat *stat, UINT32 usedSize, UINT32 taskId)
{
    if ((stat == NULL) || (taskId >= TASK_NUM)) {
        return;
    }
    stat->memTotalUsed = (stat->memTotalUsed > usedSize) ? (stat->memTotalUsed - usedSize) : 0;
    stat->taskMemstats[taskId].memUsed =
        (stat->taskMemstats[taskId].memUsed > usedSize) ? (stat->taskMemstats[taskId].memUsed - usedSize) : 0;
}

#define OS_MEM_ADD_USED(stat, usedSize, taskId) OsMemstatTaskUsedInc(stat, usedSize, taskId)
#define OS_MEM_REDUCE_USED(stat, usedSize, taskId) OsMemstatTaskUsedDec(stat, usedSize, taskId)
#else
#define OS_MEM_ADD_USED(stat, usedSize, taskId)
#define OS_MEM_REDUCE_USED(stat, usedSize, taskId)
#endif
#define OS_MEM_CLEAR(taskId)
#define OS_MEM_USAGE(taskId) 0

#endif /* PRT_BESTFIT_LITTLE_MEMSTAT_H */
