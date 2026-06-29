#ifndef PRT_BESTFIT_TASK_PRI_H
#define PRT_BESTFIT_TASK_PRI_H

#include "prt_bestfit_compat.h"
#include "prt_sys_external.h"
#include "prt_task.h"

#ifndef TASK_NUM
#define TASK_NUM (PRT_BESTFIT_BASE_CORE_TSK_LIMIT + 1)
#endif

typedef struct {
    UINT32 taskId;
    CHAR *taskName;
} PrtBestfitTaskCB;

STATIC INLINE PrtBestfitTaskCB *PrtBestfitCurrTaskGet(VOID)
{
    TskHandle taskPid;
    struct TskInfo taskInfo = {0};

    if (PRT_TaskSelf(&taskPid) != OS_OK) {
        return NULL;
    }
    if (PRT_TaskGetInfo(taskPid, &taskInfo) != OS_OK) {
        return NULL;
    }
    return (PrtBestfitTaskCB *)taskInfo.tcbAddr;
}

STATIC INLINE UINT32 PRT_CurTaskIDGet(VOID)
{
    TskHandle taskPid;
    UINT32 taskId;

    if (PRT_TaskSelf(&taskPid) != OS_OK) {
        return TASK_NUM - 1;
    }
    taskId = GET_HANDLE(taskPid);
    return (taskId < PRT_BESTFIT_BASE_CORE_TSK_LIMIT) ? taskId : (TASK_NUM - 1);
}

#endif /* PRT_BESTFIT_TASK_PRI_H */
