/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-15
 * Description: This file populates resource table for UniProton, for use by the Linux host.
 */

#include "prt_typedef.h"
#include "prt_hwi.h"
#include "rpmsg_backend.h"
void (*g_rpmsg_ipi_handler)(void);

static void IrqHandler(void)
{
    if (g_rpmsg_ipi_handler)
        g_rpmsg_ipi_handler();
}

U32 RpmsgHwiInit(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(OS_OPENAMP_NOTIFY_HWI_NUM, OS_OPENAMP_NOTIFY_HWI_PRIO, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_OPENAMP_NOTIFY_HWI_NUM, (HwiProcFunc)IrqHandler, 0);
    if (ret != OS_OK) {
        return ret;
    }

#if (OS_GIC_VER == 3)
    ret = PRT_HwiEnable(OS_OPENAMP_NOTIFY_HWI_NUM);
    if (ret != OS_OK) {
        return ret;
    }
#endif

    return OS_OK;
}