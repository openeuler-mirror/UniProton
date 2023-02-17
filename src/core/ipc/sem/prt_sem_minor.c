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

/*
 * 描述：获取信号量详细信息。
 */
OS_SEC_L4_TEXT U32 PRT_SemGetInfo(SemHandle semHandle, struct SemInfo *semInfo)
{
    struct TagSemCb *semGet = NULL;
    uintptr_t intSave;

    if (semInfo == NULL) {
        return OS_ERRNO_SEM_INFO_NULL;
    }

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }
    semGet = GET_SEM(semHandle);

    intSave = OsIntLock();
    if (semGet->semStat == OS_SEM_UNUSED) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_INVALID;
    }

    semInfo->owner = semGet->semOwner;
    semInfo->count = semGet->semCount;
    semInfo->mode = semGet->semMode;
    semInfo->type = (GET_SEM_TYPE(semGet->semType) == SEM_TYPE_COUNT) ? SEM_TYPE_COUNT : SEM_TYPE_BIN;

    OsIntRestore(intSave);
    return OS_OK;
}

/*
 * 描述：获取指定信号量的计数
 */
OS_SEC_L4_TEXT U32 PRT_SemGetCount(SemHandle semHandle, U32 *semCnt)
{
    U32 ret;
    struct SemInfo semInfo = {0};

    if (semCnt == NULL) {
        return OS_ERRNO_SEM_COUNT_GET_PTR_NULL;
    }

    ret = PRT_SemGetInfo(semHandle, &semInfo);
    if (ret == OS_OK) {
        *semCnt = semInfo.count;
    }

    return ret;
}

/*
 * 描述：获取阻塞在指定核内信号量的任务PID清单
 */
OS_SEC_L4_TEXT U32 PRT_SemGetPendList(SemHandle semHandle, U32 *tskCnt, U32 *pidBuf, U32 bufLen)
{
    uintptr_t intSave;
    U32 taskCount = 0;
    U32 len = (bufLen / sizeof(U32));
    struct TagTskCb *tskCb = NULL;
    struct TagSemCb *semCb = NULL;

    if (tskCnt == NULL) {
        return OS_ERRNO_SEM_INPUT_ERROR;
    }

    *tskCnt = 0;
    if ((pidBuf == NULL) || (len == 0)) {
        return OS_ERRNO_SEM_INPUT_ERROR;
    }

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }

    semCb = GET_SEM(semHandle);
    intSave = OsIntLock();
    if (semCb->semStat == OS_SEM_UNUSED) {
        OsIntRestore(intSave);
        return OS_ERRNO_SEM_INVALID;
    }

    LIST_FOR_EACH(tskCb, &semCb->semList, struct TagTskCb, pendList) {
        if (taskCount < len) {
            pidBuf[taskCount] = tskCb->taskPid;
        }
        taskCount++;
    }

    *tskCnt = taskCount;

    OsIntRestore(intSave);

    if (taskCount > len) {
        return OS_ERRNO_SEM_INPUT_BUF_NOT_ENOUGH;
    }

    return OS_OK;
}
