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
 * Description: tick模块的模块内头文件
 */
#ifndef PRT_TICK_INTERNAL_H
#define PRT_TICK_INTERNAL_H

#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_tick_external.h"
#include "prt_task_external.h"
#include "prt_hwi_external.h"
#include "prt_swtmr_external.h"
#include "prt_cpup_external.h"
#include "prt_hook_external.h"
#include "prt_irq_external.h"

/*
 * 模块内宏定义
 */
#define OS_SWTMR_SCAN()                \
    do {                               \
        if (g_swtmrScanHook != NULL) { \
            g_swtmrScanHook();         \
        }                              \
    } while (0)

#define OS_TICK_TASK_ENTRY()           \
    do {                               \
        if (g_tickTaskEntry != NULL) { \
            g_tickTaskEntry();         \
        }                              \
    } while (0)

#define TSKMON_TICK_RUN()           \
    do {                            \
        if (g_tskMonHook != NULL) { \
            g_tskMonHook();         \
        }                           \
    } while (0)

#define TICK_USER_HOOK_RUN()         \
    do {                             \
        if (g_tickUsrHook != NULL) { \
            g_tickUsrHook();         \
        }                            \
    } while (0)

/*
 * 模块内数据结构定义
 */
extern TickHandleFunc g_tickUsrHook;

#endif /* PRT_TICK_INTERNAL_H */
