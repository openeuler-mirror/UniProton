/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-12-04
 * Description: memInfo命令行实现
 */

#include "shcmd.h"

extern U32 g_memTotalSize;
extern U32 g_memUsage;
extern U32 g_memPeakUsage;
extern uintptr_t g_memStartAddr;

static U32 OsShellGetMemUsage(U32 usedSize, U32 totalSize) {
    U64 usedSizeRate = 10000 * usedSize;
    return (U32)(usedSizeRate / totalSize);
}

int OsShellCmdMemInfo(int argc, const char **argv)
{
    PRINTK("PtNo.  ARITHMETIC        StartAddr         EndAddr           TotalSize   "
        "UsedSize    FreeSize    PeakSize    MemUsage  \n");
    PRINTK("-----  ----------------  ----------------  ----------------  ----------  "
        "----------  ----------  ----------  ----------\n");

    U32 memUsage = OsShellGetMemUsage(g_memUsage, g_memTotalSize);
    PRINTK("0      MEM_ARITH_FSC     %#-16lx  %#-16lx  %-10u  "
        "%-10u  %-10u  %-10u  %-2u.%-2u%%\n",
        g_memStartAddr, g_memStartAddr + g_memTotalSize - 1, g_memTotalSize,
        g_memUsage, (g_memTotalSize - g_memUsage), g_memPeakUsage, (memUsage / 100), (memUsage % 100));
    return 0;
}

SHELLCMD_ENTRY(memInfo_shellcmd, CMD_TYPE_EX, "memInfo", 0, (CmdCallBackFunc)OsShellCmdMemInfo);