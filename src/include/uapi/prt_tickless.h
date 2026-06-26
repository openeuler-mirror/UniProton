/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Description: Tickless adapter header. Preserves the LiteOs OsTickless* flow on
 *              top of the native UniProton tickless kernel interface
 *              (PRT_TickLessCountGet / PRT_TickCountUpdate).
 */
#ifndef PRT_TICKLESS_H
#define PRT_TICKLESS_H

#include "prt_typedef.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(OS_OPTION_TICKLESS)

#define OS_TICKLESS_UPDATE_DEFER 0
#define OS_TICKLESS_UPDATE_DONE  1

struct TicklessOps {
    void (*start)(U32 sleepTicks);
    U32 (*update)(U32 hwiNum);
};

void PRT_TicklessOpsReg(const struct TicklessOps *ops);
/* Tell the adapter the board's tick IRQ number so OsTicklessUpdate can tell a
 * tick one-shot wake (compensate sleepTicks-1) from an early wake by another
 * IRQ (compensate elapsed cycles). Mirrors LiteOs OS_TICK_INT_NUM. */
void PRT_TicklessSetTickIrqNum(U32 irqNum);
U32 OsTicklessGetSleepTicks(void);
void OsTicklessOpen(void);
void OsTicklessUpdate(U32 hwiNum);
void OsTicklessDone(void);

#endif /* OS_OPTION_TICKLESS */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_TICKLESS_H */
