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
 * Description: sem命令行实现
 */

#include "shcmd.h"
#include "prt_sem.h"
#include "prt_sem_external.h"
#include "prt_task_external.h"

extern U16 g_maxSem;

static void ShowAllSemInfo()
{
    struct TagSemCb *semGet = NULL;
    struct SemInfo semInfo;

    PRINTK("semId         count         owner         mode         type\n");
    PRINTK("-----------------------------------------------------------\n");
    for (int i = 0; i < g_maxSem; i++) {
        semGet = GET_SEM(i);
        if (semGet->semStat == OS_SEM_UNUSED) {
            continue;
        }

        PRT_SemGetInfo(i, &semInfo);
        PRINTK("%-6u        %-6u        0x%-10x  %-6u       %-6u\n",
            i, semInfo.count, semInfo.owner, semInfo.mode, semInfo.type);
    }

    return;
}

static void ShowSemInfo(U32 semId)
{
    struct TagSemCb *semGet = GET_SEM(semId);
    if (semGet->semStat == OS_SEM_UNUSED) {
        PRINTK("Sem is unused.\n"); 
        return;
    }

    U32 tskCnt = 0;
    U32 bufLen = OS_MAX_TCB_NUM * sizeof(U32);
    U32 *pidBuf = malloc(bufLen);
    if (pidBuf == NULL) {
        return;
    }

    PRT_SemGetPendList(semId, &tskCnt, pidBuf, bufLen);

    PRINTK("Sem%u PendList\n", semId);
    PRINTK("--------------\n");
    for (int i = 0; i < tskCnt; i++) {
        PRINTK("0x%x\n", pidBuf[i]);
    }
    free(pidBuf);

    return;
}

UINT32 OsShellCmdSem(UINT32 argc, const CHAR **argv)
{
    if (argc == 0) {
        ShowAllSemInfo();
        return OS_OK;
    }

    if (!strcmp("--help", argv[0]) || argc > 1) {
        PRINTK("\nUsage: sem [semId]\n");
        return OS_OK;
    }

    char *endptr = NULL;
    U32 semId = strtoul(argv[0], &endptr, 0);
    if (endptr == NULL || endptr == argv[0] || *endptr != '\0') {
        PRINTK("Invalid sem id.\n");
        return OS_ERROR;
    }

    if (semId >= g_maxSem) {
        PRINTK("sem id out of range.\n");
        return OS_ERROR;
    }

    ShowSemInfo(semId);

    return OS_OK;
}

SHELLCMD_ENTRY(sem_shellcmd, CMD_TYPE_EX, "sem", 0, (CmdCallBackFunc)OsShellCmdSem);
