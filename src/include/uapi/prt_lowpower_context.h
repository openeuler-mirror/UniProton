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
 * Description: Low-power register context save/restore header, ported from
 *              LiteOS los_deepsleep_pri.h (OsSRSaveRegister/OsSRRestoreRegister).
 */
#ifndef PRT_LOWPOWER_CONTEXT_H
#define PRT_LOWPOWER_CONTEXT_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define LOWPOWER_COLD_RESET        0U
#define LOWPOWER_RUNSTOP_RESET     1U
#define LOWPOWER_DEEP_SLEEP_RESET  2U

extern U64 g_lowpowerSaveSRContext[34];
extern U64 g_lowpowerSaveAR[3];
extern volatile U32 g_lowpowerResumeFromImg;

void OsLowpowerContextInit(void);
void OsLowpowerSaveRegister(void);
void OsLowpowerRestoreRegister(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_LOWPOWER_CONTEXT_H */
