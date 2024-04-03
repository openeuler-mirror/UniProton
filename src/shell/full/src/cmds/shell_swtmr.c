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
 * Create: 2024-3-12
 * Description: swtmr命令行实现
 */

#include "shcmd.h"
#include "prt_swtmr_external.h"

static void PrintSwtmrInfoTitle()
{
    PRINTK("swtmrId    state    mode    interval    remainMs\n");
    PRINTK("------------------------------------------------\n");
}

static void PrintSwtmrInfo(U32 timerId, struct SwTmrInfo *info, U32 ret)
{
    if (ret == OS_OK) {
        PRINTK("%-7u    %-5u    %-4u    %-8u    %u\n",
            timerId, info->state, info->mode, info->interval, info->remainMs);
    } else if (ret == OS_ERRNO_SWTMR_NOT_CREATED) {
        PRINTK("%-7u    %-5s    %-4s    %-8s    %s\n",
            timerId, "free", "NA", "NA", "NA");
    }
}

static void ShowAllSwtmrInfo()
{
    U32 ret;

    if (g_swTmrMaxNum == 0) {
        PRINTK("swtmr is not enable.\n");
        return;
    }

    struct SwTmrInfo info = {0};

    PrintSwtmrInfoTitle();
    for (int timerId = 0; timerId < g_swTmrMaxNum; timerId++) {
        ret = PRT_SwTmrInfoGet(OS_SWTMR_INDEX_2_ID(timerId), &info);
        PrintSwtmrInfo(timerId, &info, ret);
    }

    return;
}

static void ShowSwtmrInfo(U32 timerId)
{
    U32 ret;

    if (g_swTmrMaxNum == 0) {
        PRINTK("swtmr is not enable.\n");
        return;
    }

    struct SwTmrInfo info = {0};

    PrintSwtmrInfoTitle();
    ret = PRT_SwTmrInfoGet(OS_SWTMR_INDEX_2_ID(timerId), &info);
    PrintSwtmrInfo(timerId, &info, ret);

    return;
}

UINT32 OsShellCmdSwtmr(UINT32 argc, const CHAR **argv)
{
    if (argc == 0) {
        ShowAllSwtmrInfo();
        return OS_OK;
    }

    if (!strcmp("--help", argv[0]) || argc > 1) {
        PRINTK("\nUsage: swtmr [ID]\n");
        return OS_OK;
    }

    char *endptr = NULL;
    U32 swtmrId = strtoul(argv[0], &endptr, 0);
    if (endptr == NULL || endptr == argv[0] || *endptr != '\0') {
        PRINTK("Invalid swtmr id.\n");
        return OS_ERROR;
    }

    if (swtmrId >= g_swTmrMaxNum) {
        PRINTK("swtmr id out of range.\n");
        return OS_ERROR;
    }

    ShowSwtmrInfo(swtmrId);

    return 0;
}

SHELLCMD_ENTRY(swtmr_shellcmd, CMD_TYPE_EX, "swtmr", 0, (CmdCallBackFunc)OsShellCmdSwtmr);
