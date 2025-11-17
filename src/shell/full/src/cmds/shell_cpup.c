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
 * Create: 2024-3-9
 * Description: cpup命令行实现
 */

#include "shcmd.h"
#include "prt_cpup.h"
#include "prt_cpup_internal.h"
#include "prt_sys_external.h"
#include "securec.h"
#include "prt_task.h"

static void ShowCpupConfigInfo()
{
    if (g_cpupNow == NULL) {
        PRINTK("CPUP is not enable!\n");
        return;
    }

    bool isWarncheck = FALSE;
    if (g_cpupWarnCheck != NULL) {
        isWarncheck = TRUE;
    }

    PRINTK("sampleTime(tick): %u\n", g_ticksPerSample);
    PRINTK("tickPerSecond   : %u\n", g_tickModInfo.tickPerSecond);
    PRINTK("isWarncheck     : %s\n", isWarncheck ? "YES" : "NO");

    if (isWarncheck) {
        PRINTK("warn            : %u\n", g_cpupWarnInfo.warn);
        PRINTK("resume          : %u\n", g_cpupWarnInfo.resume);
    }

    return;
}


static void GetUsageParts(U32 usage, char*buffer, U16 bufferLength, U16 maxLength)
{
    U16 integer = usage / 100;
    U16 fractional = usage % 100;
    int ret = snprintf_s(buffer, bufferLength, maxLength, "%d.%02d", integer, fractional);
    if (ret < 0) {
        PRINTK("Error: Function failed - possible truncation or constraint violation\n");
        buffer[0] = '\0';
        return;
    }
}

static void ShowCpuUsage()
{
    const char *name = NULL;
    char percentBuf[8];
    char *taskNameBuf = NULL;
    const char *percent = "%%";     // 百分号无法打印，暂时写成字符串
    U16 bufferLength = 0;
    if (g_cpupNow == NULL) {
        PRINTK("CPUP is not enable!\n");
        return;
    }
    U32 outNum = 0;
    struct CpupThread *cpup = malloc(sizeof(struct CpupThread) * OS_MAX_TCB_NUM);
    if (cpup == NULL) {
        return;
    }
    PRT_CpupThread(OS_MAX_TCB_NUM, cpup, &outNum);
    PRINTK("taskId  \ttaskName\t\tCpuUsage\n");
    PRINTK("--------------- ----------------------- ----------\n");
    for (int i = 0; i < outNum; i++) {
        GetUsageParts((U32)(cpup[i].usage), percentBuf, sizeof (percentBuf), sizeof (percentBuf)-1);
        PRT_TaskGetName(cpup[i].id, &taskNameBuf);
        name = (cpup[i].id == OS_CPUP_INT_ID) ? "Interrupt" : (const char*)taskNameBuf;
        bufferLength = strlen (percentBuf);
        if (bufferLength == strlen ("0.00")) {
            PRINTK("0x%-8x\t%-20s\t  %-4s%s\n", cpup[i].id, name, percentBuf, percent);
        } else if (bufferLength == strlen ("00.00")) {
            PRINTK("0x%-8x\t%-20s\t %-5s%s\n", cpup[i].id, name, percentBuf, percent);
        } else {
            PRINTK("0x%-8x\t%-20s\t%-6s%s\n", cpup[i].id, name, percentBuf, percent);
        }
    }
    PRINTK("--------------- ----------------------- ----------\n");
    GetUsageParts(PRT_CpupNow(), percentBuf, sizeof (percentBuf), sizeof (percentBuf)-1);
    PRINTK("Total CpuUsage: %s%s\n", percentBuf, percent);
    free(cpup);
    return;
}

UINT32 OsShellCmdCpup(UINT32 argc, const CHAR **argv)
{
    if (argc == 0) {
        ShowCpuUsage();
        return OS_OK;
    }

    if (!strcmp("-i", argv[0])) {
        ShowCpupConfigInfo();
        return OS_OK;
    }

    PRINTK("\nUsage: cpup [-i]\n");

    return OS_OK;
}

SHELLCMD_ENTRY(cpup_shellcmd, CMD_TYPE_EX, "cpup", 0, (CmdCallBackFunc)OsShellCmdCpup);