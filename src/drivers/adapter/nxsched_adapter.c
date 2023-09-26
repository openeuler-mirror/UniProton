#include "prt_buildef.h"
#include "nuttx/sched.h"

#include "prt_task_external.h"

FAR struct filelist *nxsched_get_files_from_tcb(FAR struct TagTskCb *tcb)
{
#if defined(OS_OPTION_NUTTX_VFS)
    return &(tcb->tskFileList);
#else
    return NULL;
#endif
}

FAR struct filelist *nxsched_get_files(void)
{
    return nxsched_get_files_from_tcb(RUNNING_TASK);
}

FAR struct TagTskCb *nxsched_self(void)
{
    return RUNNING_TASK;
}

void nxsched_foreach(nxsched_foreach_t handler, FAR void *arg)
{
    uintptr_t intSave;
    intSave = OsIntLock();
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_RUNNING) {
            continue;
        }
        handler(&g_tskCbArray[i], arg);
    }
    handler(RUNNING_TASK, arg);
    OsIntRestore(intSave);
}