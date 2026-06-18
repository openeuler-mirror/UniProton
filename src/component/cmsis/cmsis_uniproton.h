/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2026-06-16
 * Description: CMSIS-RTOS adapter common definitions for UniProton.
 */

#ifndef CMSIS_UNIPROTON_H
#define CMSIS_UNIPROTON_H

#include "cmsis_os.h"
#include "prt_clk.h"
#include "prt_event.h"
#include "prt_mem.h"
#include "prt_queue.h"
#include "prt_sem.h"
#include "prt_sys_external.h"
#include "prt_task.h"
#include "prt_task_external.h"
#include "prt_tick.h"
#include "prt_timer.h"
#include "prt_timer_external.h"
#include "prt_typedef.h"
#include "securec.h"

#ifndef CMSIS_OS_VER
#error CMSIS_OS_VER must be 1 or 2
#endif

#define CMSIS_DEFAULT_PRIO       25

bool CmsisKernelStarted(void);
bool CmsisKernelLocked(void);
bool CmsisPrioIsValid(int32_t priority);
U16 CmsisPrioToPrt(int32_t priority);
int32_t CmsisPrioFromPrt(TskPrior priority);
U32 CmsisMsToTick(U32 millisec);
U32 CmsisTickToMs(U32 ticks);
U32 CmsisCreateTask(TskHandle *taskId, TskEntryFunc entry, uintptr_t arg0, uintptr_t arg1,
    const char *name, U32 stackSize, U16 prio);

#endif
