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
 * Description: CPU占用率模块的C文件
 */
#include "prt_cpup_internal.h"

OS_SEC_BSS struct TagOsCpupWarnInfo g_cpupWarnInfo;

/*
 * 描述：初始化CPUP告警模块
 */
OS_SEC_L4_TEXT void OsCpupWarnInit(void)
{
    g_cpupWarnCheck = OsCpupWarn;
}

/*
 * 描述：CPUP告警，调用用户注册的钩子函数上报信息通知用户
 */
OS_SEC_L2_TEXT void OsCpupWarn(void)
{
    struct CpupWarnInfo warnInfo;
    U32 cpup;
    static bool warn = FALSE;

    cpup = OsCpupGet();
    if (cpup > g_cpupWarnInfo.warn) { /* 当超过阀值时，发出告警信息。 */
        /* 当超过阈值时，发出告警信息。 */
        if ((warn == FALSE) && (g_hookCb[OS_HOOK_CPUP_WARN].sigHook != NULL)) {
            warn = TRUE;
            warnInfo.type = CPUP_INFO_TYPE_OVERLOAD;
            OS_SHOOK_ACTIVATE_PARA1(OS_HOOK_CPUP_WARN, &warnInfo);
        }
    } else if (cpup < g_cpupWarnInfo.resume) {
        if (warn == TRUE) {
            warn = FALSE;
            if (g_hookCb[OS_HOOK_CPUP_WARN].sigHook != NULL) {
                warnInfo.type = CPUP_INFO_TYPE_RECONVERT;
                OS_SHOOK_ACTIVATE_PARA1(OS_HOOK_CPUP_WARN, &warnInfo);
            }
        }
    }
}

/*
 * 描述：告警门限和告警恢复门限的设置
 */
OS_SEC_L4_TEXT U32 PRT_CpupSetWarnValue(U32 warn, U32 resume)
{
    uintptr_t intSave;

    if (!OsCpupInitIsDone()) {
        return OS_ERRNO_CPUP_NOT_INITED;
    }

    if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) == 0) {
        return OS_ERRNO_CPUP_OS_NOT_STARTED;
    }

    if (warn > CPUP_USE_RATE || warn == 0) {
        return OS_ERRNO_CPUP_INTERVAL_NOT_SUITED;
    }

    if (resume >= warn) {
        return OS_ERRNO_CPUP_RESUME_NOT_SUITED;
    }

    intSave = OsIntLock();

    g_cpupWarnInfo.warn = warn;
    g_cpupWarnInfo.resume = resume;

    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * 描述：查询告警阈值和告警恢复阈值
 */
OS_SEC_L4_TEXT U32 PRT_CpupGetWarnValue(U32 *warn, U32 *resume)
{
    uintptr_t intSave;

    if (!OsCpupInitIsDone()) {
        return OS_ERRNO_CPUP_NOT_INITED;
    }

    if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) == 0) {
        return OS_ERRNO_CPUP_OS_NOT_STARTED;
    }

    if (warn == NULL || resume == NULL) {
        return OS_ERRNO_CPUP_PTR_NULL;
    }

    intSave = OsIntLock();

    *warn = g_cpupWarnInfo.warn;
    *resume = g_cpupWarnInfo.resume;

    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * 描述：注册CPUP告警钩子函数
 */
OS_SEC_L4_TEXT U32 PRT_CpupRegWarnHook(CpupHookFunc hook)
{
    return OsShookReg(OS_HOOK_CPUP_WARN, (OsVoidFunc)hook);
}

OS_SEC_L4_TEXT U32 OsCpupWarnReg(struct CpupModInfo *modInfo)
{
    if (modInfo->warn == 0 || modInfo->warn > CPUP_USE_RATE) {
        return OS_ERRNO_CPUP_INTERVAL_NOT_SUITED;
    }

    if (modInfo->resume >= modInfo->warn) {
        return OS_ERRNO_CPUP_RESUME_VALUE_ERROR;
    }

    g_cpupWarnInfo.warn = modInfo->warn;
    g_cpupWarnInfo.resume = modInfo->resume;

    return OS_OK;
}
