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
 * Description: 错误处理函数文件
 */
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "prt_err_internal.h"

OS_SEC_L4_BSS U32 g_normalErrorNo;
OS_SEC_L4_BSS U32 g_fatalErrorNo;

/*
 * 描述：设置致命错误码
 */
OS_SEC_ALW_INLINE INLINE void OsFatalErrSet(U32 faltError)
{
    g_fatalErrorNo = faltError;
}

OS_SEC_L4_TEXT void OsErrRecordInCda(U32 errorNo)
{
    g_normalErrorNo = errorNo;
}

/* 返回上次错误，并清零 */
OS_SEC_L4_TEXT U32 OsFatalErrClr(void)
{
    U32 oldFatalErr;
    oldFatalErr = g_fatalErrorNo;
    g_fatalErrorNo = 0;
    return oldFatalErr;
}

OS_SEC_L4_TEXT void OsErrRecord(U32 errorNo)
{
    enum SysThreadType curThreadType;
    uintptr_t interruptSave;

    struct TagTskCb *taskCb = NULL;

    interruptSave = OsIntLock();
    curThreadType = OsCurThreadType();
    if (curThreadType == SYS_TASK) {
        taskCb = GET_TCB_HANDLE(RUNNING_TASK->taskPid);
        taskCb->lastErr = errorNo;
    }

    OsIntRestore(interruptSave);
}

OS_SEC_L4_TEXT void OsErrHandle(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para)
{
    OS_ERR_CALL_USR_HOOK(fileName, lineNo, errorNo, paraLen, para);

    /* 表示用户调用，回调用户钩子后返回 */
    if (lineNo != OS_ERR_MAGIC_WORD) {
        return;
    }

    OsErrRecord(errorNo);

    /* 致命错误流程:记录trace、触发异常,高8bit为03的时候为致命错误 */
    if ((errorNo & OS_ERROR_TYPE_MASK) == ERRTYPE_FATAL) {
        OsFatalErrSet(errorNo);
        /* 调用ill指令触发异常 */
        OsAsmIll();
    }
}

/*
 * 描述：错误处理函数
 */
OS_SEC_L4_TEXT U32 PRT_ErrHandle(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para)
{
    OsErrHandle(fileName, lineNo, errorNo, paraLen, para);

    return OS_OK;
}
