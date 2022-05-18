/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 信号量模块
 */
#include "prt_sem_external.h"

OS_SEC_BSS struct TagListObject g_unusedSemList;
OS_SEC_BSS struct TagSemCb *g_allSem;

OS_SEC_L4_TEXT U32 OsSemRegister(const struct SemModInfo *modInfo)
{
    if (modInfo->maxNum == 0) {
        return OS_ERRNO_SEM_REG_ERROR;
    }

    g_maxSem = modInfo->maxNum;

    return OS_OK;
}

/*
 * 描述：信号量初始化
 */
OS_SEC_L4_TEXT U32 OsSemInit(void)
{
    struct TagSemCb *semNode = NULL;
    U32 idx;
    U32 ret = OS_OK;

    /* g_maxSem在注册时已判断是否大于0，这里不需判断 */
    g_allSem = (struct TagSemCb *)OsMemAllocAlign((U32)OS_MID_SEM,
                                                  OS_MEM_DEFAULT_FSC_PT,
                                                  g_maxSem * sizeof(struct TagSemCb),
                                                  MEM_ADDR_ALIGN_004);
    if (g_allSem == NULL) {
        return OS_ERRNO_SEM_NO_MEMORY;
    }
    if (memset_s((void *)g_allSem, g_maxSem * sizeof(struct TagSemCb), 0, g_maxSem * sizeof(struct TagSemCb)) !=
        EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    INIT_LIST_OBJECT(&g_unusedSemList);
    for (idx = 0; idx < g_maxSem; idx++) {
        semNode = ((struct TagSemCb *)g_allSem) + idx;
        semNode->semId = (U16)idx;
        ListTailAdd(&semNode->semList, &g_unusedSemList);
    }

    return ret;
}

/*
 * 描述：创建一个信号量
 */
OS_SEC_L4_TEXT U32 OsSemCreate(U32 count, enum SemMode semMode, SemHandle *semHandle, U32 cookie)
{
    uintptr_t intSave;
    struct TagSemCb *semCreated = NULL;
    struct TagListObject *unusedSem = NULL;

    (void)cookie;

    if (semHandle == NULL) {
        return OS_ERRNO_SEM_PTR_NULL;
    }

    intSave = OsIntLock();

    if (ListEmpty(&g_unusedSemList)) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_ALL_BUSY;
    }

    /* 在空闲链表中取走一个控制节点 */
    unusedSem = OS_LIST_FIRST(&(g_unusedSemList));
    ListDelete(unusedSem);

    /* 获取到空闲节点对应的信号量控制块，并开始填充控制块 */
    semCreated = (GET_SEM_LIST(unusedSem));
    semCreated->semCount = count;
    semCreated->semStat = OS_SEM_USED;
    semCreated->semMode = semMode;
    semCreated->semOwner = OS_INVALID_OWNER_ID;
    semCreated->maxSemCount = OS_SEM_COUNT_MAX;

    INIT_LIST_OBJECT(&semCreated->semList);
    *semHandle = (SemHandle)semCreated->semId;

    OsIntRestore(intSave);
    return OS_OK;
}

/*
 * 描述：创建一个信号量
 */
OS_SEC_L4_TEXT U32 PRT_SemCreate(U32 count, SemHandle *semHandle)
{
    U32 ret;

    if (count > OS_SEM_COUNT_MAX) {
        return OS_ERRNO_SEM_OVERFLOW;
    }

    ret = OsSemCreate(count, SEM_MODE_FIFO, semHandle, (U32)(uintptr_t)semHandle);
    return ret;
}

/*
 * 描述：删除一个信号量
 */
OS_SEC_L4_TEXT U32 PRT_SemDelete(SemHandle semHandle)
{
    uintptr_t intSave;
    struct TagSemCb *semDeleted = NULL;

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }
    semDeleted = GET_SEM(semHandle);

    intSave = OsIntLock();

    if (semDeleted->semStat == OS_SEM_UNUSED) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_INVALID;
    }
    if (!ListEmpty(&semDeleted->semList)) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_PENDED;
    }
    semDeleted->semStat = OS_SEM_UNUSED;
    ListAdd(&semDeleted->semList, &g_unusedSemList);

    OsIntRestore(intSave);
    return OS_OK;
}
