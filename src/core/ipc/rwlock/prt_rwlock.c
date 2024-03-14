/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: pthread rwlock功能实现
 */
#include "prt_rwlock_internal.h"
#include "prt_task_external.h"

OS_SEC_ALW_INLINE INLINE bool OsRwlockPriCompare(struct TagTskCb *runTask, struct TagListObject *rwList)
{
    struct TagTskCb *task;

    if (!ListEmpty(rwList)) {
        task = GET_TCB_PEND(OS_LIST_FIRST(rwList));
        if (runTask->priority < task->priority) {
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

struct TagListObject *OsRwLockPendFindPosSub(const struct TagTskCb *runTask, const struct TagListObject *lockList)
{
    struct TagTskCb *pendedTask;
    struct TagListObject *node = NULL;

    LIST_FOR_EACH_SAFE(pendedTask, lockList, struct TagTskCb, pendList) {
        if (pendedTask->priority < runTask->priority) {
            continue;
        } else if (pendedTask->priority > runTask->priority) {
            node = &pendedTask->pendList;
            break;
        } else {
            node = pendedTask->pendList.next;
            break;
        }
    }

    return node;
}

struct TagListObject *OsRwLockPendFindPos(struct TagTskCb *runTask, struct TagListObject *lockList)
{
    struct TagListObject *node = NULL;
    struct TagTskCb *taskFirst;
    struct TagTskCb *taskLast;

    if (ListEmpty(lockList)) {
        node = lockList;
    } else {
        taskFirst = GET_TCB_PEND(OS_LIST_FIRST(lockList));
        taskLast = GET_TCB_PEND(LIST_LAST(lockList));
        if (taskFirst->priority > runTask->priority) {
            node = lockList->next;
        } else if (taskLast->priority <= runTask->priority) {
            node = lockList;
        } else {
            node = OsRwLockPendFindPosSub(runTask, lockList);
        }
    }

    return node;
}

void OsRwLockPendPre(struct TagTskCb *runTask, struct TagListObject *list, U32 timeout)
{
    OsTskReadyDel(runTask);

    TSK_STATUS_SET(runTask, OS_TSK_PEND);

    if (timeout != OS_WAIT_FOREVER) {
        TSK_STATUS_SET(runTask, OS_TSK_TIMEOUT);
        OsTskTimerAdd(runTask, timeout);
    }

    ListTailAdd(&(runTask->pendList), list);
}

void OsRwLockTaskWake(struct TagTskCb *resumedTask)
{
    ListDelete(&resumedTask->pendList);

    if (TSK_STATUS_TST(resumedTask, OS_TSK_TIMEOUT)) {
        OS_TSK_DELAY_LOCKED_DETACH(resumedTask);
    }

    TSK_STATUS_CLEAR(resumedTask, OS_TSK_TIMEOUT | OS_TSK_PEND);

    if (!TSK_STATUS_TST(resumedTask, OS_TSK_SUSPEND_READY_BLOCK)) {
        OsTskReadyAddBgd(resumedTask);
    }
}

U32 OsRwLockTryRdCheck(struct TagTskCb *runTask, pthread_rwlock_t *rwl)
{
    if ((struct TagTskCb *)(rwl->rw_owner) == runTask) {
        return EINVAL;
    }

    /* 读写锁为读模式或者初始化模式，并且当前读锁任务的优先级比第一个写锁pend任务的优先级低，
     * 当前读锁任务不能获取锁
     */
    if ((rwl->rw_count >= 0) && !OsRwlockPriCompare(runTask, &(rwl->rw_write))) {
        return EBUSY;
    }

    /* 读写锁被写锁获得，当前读锁任务不能获取锁 */
    if (rwl->rw_count < 0) {
        return EBUSY;
    }

    return OS_OK;
}

U32 OsRwlockTryWrCheck(struct TagTskCb *runTask, pthread_rwlock_t *rwl)
{
    /* 读写锁被读锁获得，当前写锁任务不能获取锁 */
    if (rwl->rw_count > 0) {
        return EBUSY;
    }

    /* 读写锁被写锁获得，当前写锁任务不能获取锁. */
    if ((rwl->rw_count < 0) && ((struct TagTskCb *)(rwl->rw_owner) != runTask)) {
        return EBUSY;
    }

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsRwLockCheck(pthread_rwlock_t *rwl)
{
    if (rwl == NULL) {
        return EINVAL;
    }

    if ((rwl->rw_magic & RWLOCK_COUNT_MASK) != RWLOCK_MAGIC_NUM) {
        return EINVAL;
    }

    if (OS_INT_ACTIVE) {
        return EINVAL;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        return EDEADLK;
    }

    return OS_OK;
}

U32 OsRwLockPendSchedule(struct TagTskCb *runTask, struct TagListObject *lockList, U32 timeout, U32 intSave)
{
    struct TagListObject *node;

    if (timeout == 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    node = OsRwLockPendFindPos(runTask, lockList);
    OsRwLockPendPre(runTask, node, timeout);
    if (timeout != OS_WAIT_FOREVER) {
        OsTskSchedule();
        PRT_HwiRestore(intSave);
        intSave = PRT_HwiLock();
        /* 判断是否是等待信号量超时 */
        if (TSK_STATUS_TST(runTask, OS_TSK_TIMEOUT)) {
            TSK_STATUS_CLEAR(runTask, OS_TSK_TIMEOUT);
            PRT_HwiRestore(intSave);
            return ETIMEDOUT;
        }
        PRT_HwiRestore(intSave);
    } else {
        OsTskSchedule();
        PRT_HwiRestore(intSave);
    }

    return OS_OK;
}

U32 OsRwLockRdPend(pthread_rwlock_t *rwl, U32 timeout, U32 rwType)
{
    U32 ret;
    U32 intSave;
    struct TagTskCb *runTask = NULL;

    intSave = PRT_HwiLock();

    ret = OsRwLockCheck(rwl);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return ret;
    }

    runTask = (struct TagTskCb *)RUNNING_TASK;

    if (rwType == RWLOCK_TRYRD) {
        ret = OsRwLockTryRdCheck(runTask, rwl);
        if (ret != OS_OK) {
            PRT_HwiRestore(intSave);
            return ret;
        }
    }

     /*
      * 读写锁为读模式或者初始化模式，并且当前读锁任务的优先级比第一个写锁pend任务的优先级高，
      * 当前读锁任务能获取锁
      */
    if (rwl->rw_count >= 0) {
        if (OsRwlockPriCompare(runTask, &(rwl->rw_write))) {
            if (rwl->rw_count == S32_MAX) {
                PRT_HwiRestore(intSave);
                return EINVAL;
            }
            rwl->rw_count++;
            PRT_HwiRestore(intSave);
            return OS_OK;
        }
    }

    if ((struct TagTskCb *)(rwl->rw_owner) == runTask) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    return OsRwLockPendSchedule(runTask, &(rwl->rw_read), timeout, intSave);
}

U32 OsRwLockWrPend(pthread_rwlock_t *rwl, U32 timeout, U32 rwType)
{
    U32 ret;
    U32 intSave;
    struct TagTskCb *runTask;

    intSave = PRT_HwiLock();

    ret = OsRwLockCheck(rwl);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return ret;
    }

    runTask = (struct TagTskCb *)RUNNING_TASK;
    if (rwType == RWLOCK_TRYWR) {
        ret = OsRwlockTryWrCheck(runTask, rwl);
        if (ret != OS_OK) {
            PRT_HwiRestore(intSave);
            return ret;
        }
    }

    if (rwl->rw_count == 0) {
        rwl->rw_count = -1;
        rwl->rw_owner = (void *)runTask;
        PRT_HwiRestore(intSave);
        return OS_OK;
    }

    /* 如果读写锁被自身获取，只能获取一次. */
    if ((rwl->rw_count < 0) && ((struct TagTskCb *)(rwl->rw_owner) == runTask)) {
        if (rwl->rw_count == S32_MIN) {
            PRT_HwiRestore(intSave);
            return EINVAL;
        }
        PRT_HwiRestore(intSave);
        return OS_OK;
    }

    return OsRwLockPendSchedule(runTask, &(rwl->rw_write), timeout, intSave);
}

U32 OsRwLockGetMode(struct TagListObject *readList, struct TagListObject *writeList)
{
    bool isReadEmpty = ListEmpty(readList);
    bool isWriteEmpty = ListEmpty(writeList);
    if (isReadEmpty && isWriteEmpty) {
        return RWLOCK_NONE_MODE;
    }
    if (!isReadEmpty && isWriteEmpty) {
        return RWLOCK_READ_MODE;
    }
    if (isReadEmpty && !isWriteEmpty) {
        return RWLOCK_WRITE_MODE;
    }

    struct TagTskCb *pendedReadTask = GET_TCB_PEND(OS_LIST_FIRST(readList));
    struct TagTskCb *pendedWriteTask = GET_TCB_PEND(OS_LIST_FIRST(writeList));
    if (pendedWriteTask->priority <= pendedReadTask->priority) {
        return RWLOCK_WRITEFIRST_MODE;
    }
    return RWLOCK_READFIRST_MODE;
}

U32 OsRwLockPost(pthread_rwlock_t *rwl, bool *needSched)
{
    U32 rwlockMode;
    struct TagTskCb *resumedTask = NULL;
    U32 pendedWriteTaskPri = 0;

    rwl->rw_count = 0;
    rwl->rw_owner = NULL;
    rwlockMode = OsRwLockGetMode(&(rwl->rw_read), &(rwl->rw_write));
    if (rwlockMode == RWLOCK_NONE_MODE) {
        return OS_OK;
    }

    /* 唤醒第一个被pend的写锁任务 */
    if ((rwlockMode == RWLOCK_WRITE_MODE) || (rwlockMode == RWLOCK_WRITEFIRST_MODE)) {
        resumedTask = GET_TCB_PEND(OS_LIST_FIRST(&(rwl->rw_write)));
        rwl->rw_count = -1;
        rwl->rw_owner = (void *)resumedTask;
        OsRwLockTaskWake(resumedTask);
        if (needSched != NULL) {
            *needSched = TRUE;
        }
        return OS_OK;
    }

    /* 唤醒被pend的读锁任务 */
    if (rwlockMode == RWLOCK_READFIRST_MODE) {
        pendedWriteTaskPri = GET_TCB_PEND(OS_LIST_FIRST(&(rwl->rw_write)))->priority;
    }

    resumedTask = GET_TCB_PEND(OS_LIST_FIRST(&(rwl->rw_read)));
    rwl->rw_count = 1;
    OsRwLockTaskWake(resumedTask);
    while (!ListEmpty(&(rwl->rw_read))) {
        resumedTask = GET_TCB_PEND(OS_LIST_FIRST(&(rwl->rw_read)));
        if ((rwlockMode == RWLOCK_READFIRST_MODE) && (resumedTask->priority >= pendedWriteTaskPri)) {
            break;
        }

        if (rwl->rw_count == S32_MAX) {
            return EINVAL;
        }

        rwl->rw_count++;
        OsRwLockTaskWake(resumedTask);
    }

    if (needSched != NULL) {
        *needSched = TRUE;
    }
    return OS_OK;
}

U32 OsRwLockUnlock(pthread_rwlock_t *rwl, bool *needSched)
{
    struct TagTskCb *runTask;

    if (rwl == NULL) {
        return EINVAL;
    }
    if ((rwl->rw_magic & RWLOCK_COUNT_MASK) != RWLOCK_MAGIC_NUM) {
        return EINVAL;
    }

    if (rwl->rw_count == 0) {
        return EPERM;
    }

    runTask = RUNNING_TASK;
    if ((rwl->rw_count < 0) && ((struct TagTskCb *)(rwl->rw_owner) != runTask)) {
        return EPERM;
    }

    if (rwl->rw_count > 1) {
        rwl->rw_count--;
        return OS_OK;
    }

    if (rwl->rw_count < -1) {
        rwl->rw_count++;
        return OS_OK;
    }

    return OsRwLockPost(rwl, needSched);
}
