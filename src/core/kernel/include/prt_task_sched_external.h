/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-29
 * Description: 任务模块的内部头文件
 */

#ifndef PRT_TASK_SCHED_EXTERNAL_H
#define PRT_TASK_SCHED_EXTERNAL_H

#if defined(OS_OPTION_SMP)
#include "prt_sched_external.h"
#endif
#include "prt_atomic.h"

#if defined(OS_OPTION_SMP) && (OS_MAX_CORE_NUM > 1)
// 尝试根据PID获取任务所属RQ锁直到获取为止，涉及任务态的迁移，都是用RQ锁保证互斥
extern void OsSpinLockTaskRq(struct TagTskCb* taskCB);

extern bool OsTrySpinLockTaskRq(struct TagTskCb *taskCB);

/* running任务持有自己的rq锁无需判断，中途不可能换核 */
extern struct TagOsRunQue *OsSpinLockRunTaskRq(void);

extern void OsSpinUnLockRunTaskRq(struct TagOsRunQue *runQue);

// 释放PID所属RQ锁
extern void OsSpinUnlockTaskRq(struct TagTskCb *taskCB);

extern U32 OsTryLockTaskOperating(U32 operate, struct TagTskCb *taskCB, uintptr_t *intSave);
#else
OS_SEC_ALW_INLINE INLINE void OsSpinLockTaskRq(struct TagTskCb* taskCB)
{
    (void)taskCB;
}

OS_SEC_ALW_INLINE INLINE struct TagOsRunQue *OsSpinLockRunTaskRq(void)
{
    return NULL;
}

OS_SEC_ALW_INLINE INLINE void OsSpinUnlockTaskRq(struct TagTskCb* taskCB)
{
    (void)taskCB;
}

OS_SEC_ALW_INLINE INLINE void OsSpinUnLockRunTaskRq(struct TagOsRunQue *runQue)
{
    (void)runQue;
}

OS_SEC_ALW_INLINE INLINE U32 OsTryLockTaskOperating(U32 operate, struct TagTskCb *taskCB, uintptr_t *intSave)
{
    (void)operate;
    (void)taskCB;
    *intSave = OsIntLock();
    return OS_OK;
}
#endif
#endif