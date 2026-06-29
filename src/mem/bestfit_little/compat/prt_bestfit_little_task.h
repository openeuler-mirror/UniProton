#ifndef PRT_BESTFIT_LITTLE_TASK_H
#define PRT_BESTFIT_LITTLE_TASK_H

#include "prt_bestfit_little_compat.h"
#include "prt_bestfit_little_config.h"
#include "prt_sys_external.h"
#include "prt_task.h"

#ifndef TASK_NUM
#define TASK_NUM (LOSCFG_BASE_CORE_TSK_LIMIT + 1)
#endif

STATIC INLINE VOID *OsCurrTaskGet(VOID)
{
    TskHandle taskPid;
    struct TskInfo taskInfo = {0};

    if (PRT_TaskSelf(&taskPid) != OS_OK) {
        return NULL;
    }

    if (PRT_TaskGetInfo(taskPid, &taskInfo) != OS_OK) {
        return NULL;
    }

    return taskInfo.tcbAddr;
}

STATIC INLINE UINT32 LOS_CurTaskIDGet(VOID)
{
    TskHandle taskPid;
    UINT32 taskId;

    if (PRT_TaskSelf(&taskPid) != OS_OK) {
        return TASK_NUM - 1;
    }

    taskId = GET_HANDLE(taskPid);
    return (taskId < LOSCFG_BASE_CORE_TSK_LIMIT) ? taskId : (TASK_NUM - 1);
}

#endif /* PRT_BESTFIT_LITTLE_TASK_H */
