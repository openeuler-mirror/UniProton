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

extern uintptr_t g_memTotalSize;
extern uintptr_t g_memUsage;
extern uintptr_t g_memPeakUsage;
extern uintptr_t g_memStartAddr;

static double OsShellGetMemUsageRate(uintptr_t usedSize, uintptr_t totalSize) {
    return (usedSize * 100.0 / totalSize);
}

int OsShellCmdMemInfo(int argc, const char **argv)
{
    if (argc > 0) {
        PRINTK("\nUsage: memInfo\n");
        return OS_ERROR;
    }

    PRINTK("PtNo.  ARITHMETIC        StartAddr         EndAddr           TotalSize   "
        "UsedSize    FreeSize    PeakSize    MemUsage  \n");
    PRINTK("-----  ----------------  ----------------  ----------------  ----------  "
        "----------  ----------  ----------  ----------\n");

    double memUsageRate = OsShellGetMemUsageRate(g_memUsage, g_memTotalSize);

    if (sizeof(uintptr_t) == 4) {
        PRINTK("0      MEM_ARITH_FSC     %#-16x  %#-16x  %-10u  "
            "%-10u  %-10u  %-10u  %.2fpercent\n",
            g_memStartAddr, g_memStartAddr + g_memTotalSize - 1, g_memTotalSize,
            g_memUsage, (g_memTotalSize - g_memUsage), g_memPeakUsage, memUsageRate);
    } else {
        PRINTK("0      MEM_ARITH_FSC     %#-16lx  %#-16lx  %-10lu  "
            "%-10lu  %-10lu  %-10lu  %.2fpercent\n",
            g_memStartAddr, g_memStartAddr + g_memTotalSize - 1, g_memTotalSize,
            g_memUsage, (g_memTotalSize - g_memUsage), g_memPeakUsage, memUsageRate);
    }

    return OS_OK;
}

SHELLCMD_ENTRY(memInfo_shellcmd, CMD_TYPE_EX, "memInfo", 0, (CmdCallBackFunc)OsShellCmdMemInfo);