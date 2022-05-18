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
 * Create: 2009-10-05
 * Description: 异常模块的内部头文件
 */
#ifndef PRT_EXC_EXTERNAL_H
#define PRT_EXC_EXTERNAL_H

#include "prt_task.h"
#include "prt_sys.h"
#include "prt_kexc_external.h"

/* 异常类型定义 */
/* 内核进程下异常 */
#define EXC_IN_HWI      0
#define EXC_IN_TICK     1
#define EXC_IN_TASK     3
#define EXC_IN_SYS_BOOT 4
#define EXC_IN_SYS      5

/*
 * 模块间typedef声明
 */
typedef void (*ExcTaskInfoFunc)(TskHandle *threadId, struct TskInfo *taskInfo);

/*
 * 模块间全局变量声明
 */
// 异常时获取当前任务的信息
extern ExcTaskInfoFunc g_excTaskInfoGet;
extern void OsExcDispatch(U32 arg);

#endif /* PRT_EXC_EXTERNAL_H */
