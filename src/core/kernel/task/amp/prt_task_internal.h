/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: task模块的模块内头文件
 */
#ifndef PRT_TASK_INTERNAL_H
#define PRT_TASK_INTERNAL_H

#include "prt_task_external.h"
#include "prt_signal_external.h"
#include "prt_asm_cpu_external.h"

/*
 * 模块内宏定义
 */
#define OS_TSK_STACK_MAGIC g_tskModInfo.magicWord
#define OS_TSK_STACK_CFG_BY_USER 1
#define OS_TSK_STACK_CFG_BY_SYS 0

/*
 * 模块内全局变量声明
 */
extern struct TagListObject g_tskCbFreeList;
extern struct TagListObject g_tskRecyleList;

/*
 * 模块内函数声明
 */
extern void *OsTskMemAlloc(U32 size);
extern void OsTskIdleBgd(void);
extern U32 OsTaskDelStatusCheck(struct TagTskCb *taskCb);
extern void OsTskRecycle(void);
extern void OsTskStackInit(U32 stackSize, uintptr_t topStack);
extern U32 OsTaskCreateParaCheck(const TskHandle *taskPid, struct TskInitParam *initParam);
void OsTskCreateTcbInit(uintptr_t stackPtr, struct TskInitParam *initParam, uintptr_t topStackAddr,
    uintptr_t curStackSize, struct TagTskCb *taskCb);
U32 OsTaskCreateRsrcInit(U32 taskId, struct TskInitParam *initParam, struct TagTskCb *taskCb, uintptr_t **topStackOut,
    uintptr_t *curStackSize);

OS_SEC_ALW_INLINE INLINE void OsMoveTaskToReady(struct TagTskCb *taskCb)
{
    if (TSK_STATUS_TST(taskCb, OS_TSK_DELAY_INTERRUPTIBLE)) {
        /* 可中断delay, 属于定时等待的任务时候，去掉其定时等待标志位*/
        if (TSK_STATUS_TST(taskCb, OS_TSK_TIMEOUT)) {
            OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        }
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT | OS_TSK_DELAY_INTERRUPTIBLE);
    }

    /* If task is not blocked then move it to ready list */
    if ((taskCb->taskStatus & OS_TSK_BLOCK) == 0) {
        OsTskReadyAdd(taskCb);
#if defined(OS_OPTION_LINUX)
        KTHREAD_TSK_STATE_SET(taskCb, TASK_RUNNING);
#endif   
        if ((OS_FLG_BGD_ACTIVE & UNI_FLAG) != 0) {
            OsTskSchedule();
            return;
        }
    }
}

OS_SEC_ALW_INLINE INLINE void OsTskResRecycle(struct TagTskCb *taskCb)
{
    if (taskCb->stackCfgFlg == OS_TSK_STACK_CFG_BY_SYS) {
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_TSK, (void *)taskCb->topOfStack));
    }
}

OS_SEC_ALW_INLINE INLINE U32 OsTaskCreateChkAndGetTcb(struct TagTskCb **taskCb)
{
    OsTskRecycle();

    if (ListEmpty(&g_tskCbFreeList)) {
        return OS_ERRNO_TSK_TCB_UNAVAILABLE;
    }

    // 先获取到该控制块
    *taskCb = GET_TCB_PEND(OS_LIST_FIRST(&g_tskCbFreeList));
    // 成功，从空闲列表中移除
    ListDelete(OS_LIST_FIRST(&g_tskCbFreeList));

    return OS_OK;
}

#endif /* PRT_TASK_INTERNAL_H */
