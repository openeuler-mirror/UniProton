/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-3-2
 * Description: hwi命令行实现
 */

#include "shcmd.h"
#include "prt_irq_internal.h"

static void ShowHwiInfo(U32 hwiNum)
{
    U32 irqNum = OS_HWI2IRQ(hwiNum);
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    struct TagHwiModeForm *form = OS_HWI_MODE_ATTR(irqNum);
    PRINTK("%11u    0x%-5x %-6u\n", hwiNum, form->mode, form->prior);
#else
    PRINTK("%11u    NA     NA\n", hwiNum);
#endif

    return;
}

UINT32 OsShellCmdHwi(UINT32 argc, const CHAR **argv)
{
    (VOID)argv;

    if (argc > 0) {
        PRINTK("\nUsage: hwi\n");
        return OS_ERROR;
    }

    PRINTK("InterruptNo    Mode    Prior\n");
    PRINTK("----------------------------\n");
    for (U32 hwiNum = 0; hwiNum < OS_HWI_MAX_NUM; hwiNum++) {
        if (OS_HWI_NUM_CHECK(hwiNum)) {
            continue;
        }
        if (!OS_HWI_MODE_INV(hwiNum)) {
            ShowHwiInfo(hwiNum);
        }
    }

    return OS_OK;
}

SHELLCMD_ENTRY(hwi_shellcmd, CMD_TYPE_EX, "hwi", 0, (CmdCallBackFunc)OsShellCmdHwi);
