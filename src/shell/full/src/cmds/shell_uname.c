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
 * Description: uname命令行实现
 */

#include "shcmd.h"
#include "prt_sys.h"

UINT32 OsShellCmdUname(UINT32 argc, const CHAR **argv)
{
    (VOID)argv;
    char *version = NULL;

    if (argc > 0) {
        PRINTK("\nUsage: uname\n");
        return OS_ERROR;
    }

    version = PRT_SysGetOsVersion();
    PRINTK("%s\n", version);

    return OS_OK;
}

SHELLCMD_ENTRY(uname_shellcmd, CMD_TYPE_EX, "uname", 0, (CmdCallBackFunc)OsShellCmdUname);
