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

OS_SEC_ALW_INLINE INLINE void OsMoveTaskToReady(struct TagTskCb *taskCb)
{
    /* If task is not blocked then move it to ready list */
    if ((taskCb->taskStatus & OS_TSK_BLOCK) == 0) {
        OsTskReadyAdd(taskCb);

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
#endif /* PRT_TASK_INTERNAL_H */
