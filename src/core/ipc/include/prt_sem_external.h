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
 * Description: 信号量模块内部头文件
 */
#ifndef PRT_SEM_EXTERNAL_H
#define PRT_SEM_EXTERNAL_H

#include "prt_sem.h"
#include "prt_task_external.h"

#define OS_SEM_UNUSED 0
#define OS_SEM_USED 1

#define OS_SEM_WITH_LOCK_FLAG 1
#define OS_SEM_WITHOUT_LOCK_FLAG 0

#define GET_SEM_LIST(ptr) LIST_COMPONENT(ptr, struct TagSemCb, semList)
#define GET_SEM(semid) (((struct TagSemCb *)g_allSem) + (semid))
#define GET_SEM_TSK(semid) (((SEM_TSK_S *)g_semTsk) + (semid))
#define GET_TSK_SEM(tskid) (((TSK_SEM_S *)g_tskSem) + (tskid))

struct TagSemCb {
    /* 是否使用 OS_SEM_UNUSED/OS_SEM_USED */
    U16 semStat;
    /* 核内信号量索引号 */
    U16 semId;
    /* 当该信号量已用时，其信号量计数 */
    U32 semCount;
    /* 挂接阻塞于该信号量的任务 */
    struct TagListObject semList;
    U32 maxSemCount;

    /* Pend到该信号量的线程ID */
    U32 semOwner;
    /* 信号量唤醒阻塞任务的方式 */
    enum SemMode semMode;
};

/* 模块间全局变量声明 */
extern U16 g_maxSem;

/* 指向核内信号量控制块 */
extern struct TagSemCb *g_allSem;
#endif /* PRT_SEM_EXTERNAL_H */
