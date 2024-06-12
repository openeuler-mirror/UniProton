/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: 硬件相关的通用处理。
 */
#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_attr_external.h"

#include "prt_sys_external.h"
RESET_SEC_DATA volatile U32 g_secondaryResetFlag = 0;

INIT_SEC_L4_TEXT void OsSetValidAllCoresMask(U32 cfgPrimaryCore)
{
    if(cfgPrimaryCore >= OS_MAX_CORE_NUM) {
        return;
    }

    for (U32 coreId = 0; coreId < cfgPrimaryCore; coreId++) {
        g_validAllCoreMask &= ~(1U << coreId);
    }
}

INIT_SEC_L4_TEXT void OsHwInit(void)
{
}

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
	U64 cycle;
	OS_EMBED_ASM("MRS %0, CNTPCT_EL0" : "=r"(cycle)::"memory", "cc");
	return cycle;
}
