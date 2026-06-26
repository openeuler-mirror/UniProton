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
 * Description: Low-power register context data and init. The save/restore
 *              register routines live in prt_lowpower_context_asm.S (ported from
 *              LiteOs arch/arm64/src/runstop.S). Symbols are renamed to the
 *              PRT lowpower naming.
 */
#include "prt_lowpower_context.h"
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_attr_external.h"

#if defined(OS_ARCH_CPU64)

/* 34: X0~X30, SP, DAIF, NZCV register context for suspend/resume. */
OS_SEC_BSS __attribute__((aligned(16))) U64 g_lowpowerSaveSRContext[34];
/* 3: temporary general registers (X0, X1, X2) saved across the routine. */
OS_SEC_BSS U64 g_lowpowerSaveAR[3];

#endif

/* 0 = cold boot; non-zero means resumed from a saved image. */
OS_SEC_DATA volatile U32 g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;

void OsLowpowerContextInit(void)
{
#if defined(OS_ARCH_CPU64)
    for (U32 i = 0; i < 34; i++) {
        g_lowpowerSaveSRContext[i] = 0;
    }
    g_lowpowerSaveAR[0] = 0;
    g_lowpowerSaveAR[1] = 0;
    g_lowpowerSaveAR[2] = 0;
#endif
    g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;
}
