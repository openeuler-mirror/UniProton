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
 * Description: help命令行实现
 */

#include "shcmd.h"
#include "shell_pri.h"

UINT32 OsShellCmdHelp(UINT32 argc, const CHAR **argv)
{
    UINT32 loop = 0;
    CmdItemNode *curCmdItem = NULL;
    CmdModInfo *cmdInfo = OsCmdInfoGet();

    (VOID)argv;
    if (argc > 0) {
        PRINTK("\nUsage: help\n");
        return OS_ERROR;
    }

    PRINTK("support shell commond:\n");
    LOS_DL_LIST_FOR_EACH_ENTRY(curCmdItem, &cmdInfo->cmdList.list, CmdItemNode, list) {
        if ((loop & (8 - 1)) == 0) { /* 8 - 1:just align print */
            PRINTK("\n");
        }
        PRINTK("\t%-12s\n", curCmdItem->cmd->cmdKey);

        loop++;
    }

    return OS_OK;
}

SHELLCMD_ENTRY(help_shellcmd, CMD_TYPE_EX, "help", 0, (CmdCallBackFunc)OsShellCmdHelp);
