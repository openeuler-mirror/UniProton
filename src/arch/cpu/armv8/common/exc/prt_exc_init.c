/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: 异常处理（初始化相关）。
 */
#include "prt_exc_internal.h"
#include "prt_err_external.h"
#if defined(OS_OPTION_POWEROFF)
#include "prt_task_external.h"
#endif
#include "prt_log.h"
#include "prt_stacktrace.h"
OS_SEC_BSS bool g_isOtherCoreExc[OS_VAR_ARRAY_NUM];

#if defined(OS_OPTION_SMP)
OS_SEC_BSS uintptr_t g_uniFlagAddr[OS_MAX_CORE_NUM];
#endif
/*
 * 描述: 获取异常类型
 */
static INIT_SEC_L4_TEXT U32 OsExcGetDefExctype(uintptr_t esr)
{
    U32 excCause;
    union TagExcEsr excEsr;

    excEsr.esr = esr;

    switch ((U32)excEsr.bits.ec) {
        case OS_EXC_ESR_UNDEF_INSTR:
        /* fall through */
        case OS_EXC_ESR_UNDEF_INSTR + 1:
            excCause = OS_EXCEPT_UNDEF_INSTR;
            break;
        case OS_EXC_ESR_PC_NOT_ALIGN:
            excCause = OS_EXCEPT_PC_NOT_ALIGN;
            break;
        case OS_EXC_ESR_DATA_ABORT:
        /* fall through */
        case OS_EXC_ESR_DATA_ABORT + 1:
            excCause = OS_EXCEPT_DATA_ABORT;
            break;
        case OS_EXC_ESR_SP_INSTR:
            excCause = OS_EXCEPT_SP_NOT_ALIGN;
            break;
        default:
            excCause = OS_EXCEPT_ESR;
            break;
    }
    return excCause;
}

/*
 * 描述: 记录异常原因
 */
static INIT_SEC_L4_TEXT void OsExcSaveCause(uintptr_t esr)
{
    U32 preFatalErr;
    U32 excCause;
    struct ExcInfo *excInfo = OS_EXC_INFO_ADDR;

    if (g_isOtherCoreExc[OsGetCoreID()]) {
        excInfo->excCause = OS_EXCEPT_OTHER_CORE;
        return;
    }

    /* 记录异常类型 */
    preFatalErr = OsFatalErrClr();
    if (preFatalErr == OS_EXC_DEFAULT_EXC_TYPE) {
        excCause = OsExcGetDefExctype(esr);
    } else {
        excCause = OS_EXCEPT_FATALERROR;
    }

    excInfo->excCause = excCause;
    excInfo->fatalErrNo = preFatalErr;
    return;
}
#if defined(OS_OPTION_STACKTRACE)
INIT_SEC_L4_TEXT U32 OsExcCallstackPrintf(const struct ExcInfo *excInfo)
{
    uintptr_t list[STACKTRACE_MAX_DEPTH] = {0};
    U32 maxDepth = STACKTRACE_MAX_DEPTH;
    U32 ret;
    U32 coreId = OsGetCoreID();

    if (excInfo == NULL) {
        PRT_Printf("excInfo is NULL\n");
        return -1;
    }

    PRT_Printf("CoreId: %u trigger exception\n",coreId);

    if (g_isOtherCoreExc[OsGetCoreID()] == TRUE) {
        PRT_Printf("Other Core Trigger Exception\n");
        return OS_OK;
    }

    ret = PRT_GetStackTrace(&maxDepth, list);

    if ((ret == OS_OK) && (maxDepth > 0)) {
        ret = PRT_PrintStackTraceResult(maxDepth, list);
    } else {
        PRT_Printf("Get callstack info failed.\n");
        return -1;
    }
    
    return OS_OK;
}
#endif
/*
 * 描述: EXC钩子处理函数
 */
INIT_SEC_L4_TEXT void OsExcHookHandle(void)
{
    struct ExcInfo *excInfo = OS_EXC_INFO_ADDR;

    if (g_excModInfo.excepHook != NULL) {
        (void)g_excModInfo.excepHook(excInfo); // 目前不支持异常返回
    }

    PRT_SysReboot();
}

/*
 * 描述: FIQ异常处理
 */
OS_SEC_ALW_INLINE INLINE void OsExcFiqProc(U32 excType)
{
    struct ExcInfo excInfo = {0};
    excInfo.excCause = excType;

    if (g_excModInfo.excepHook != NULL) {
        (void)g_excModInfo.excepHook(&excInfo);
    }

    PRT_SysReboot();
}

/*
 * 描述: EXC模块的处理分发函数
 */
INIT_SEC_L4_TEXT void OsExcHandleEntry(U32 excType, struct ExcRegInfo *excRegs)
{
    U32 coreID = THIS_CORE();
    struct ExcInfo *excInfo = OS_EXC_INFO_ADDR;

    UNI_FLAG |= (OS_FLG_HWI_ACTIVE | OS_FLG_EXC_ACTIVE);
    if (excType == OS_EXCEPT_FIQ) {
        OsExcFiqProc(excType);
        return;
    }

    OsExcSaveCause(excRegs->esr);

    CUR_NEST_COUNT++;

    /* 记录异常信息 */
    OsExcSaveInfo(excInfo, excRegs);

    PRT_LogFormat(OS_LOG_EMERG, OS_LOG_F0, "[core%u] OsExcHandleEntry enter", excInfo->coreId);
    
#if defined(OS_OPTION_STACKTRACE)
    /* 打印异常任务栈 */
    OsErrRecord(OsExcCallstackPrintf(excInfo));
#endif

    /* 回调异常钩子函数 */
    OsExcHookHandle();
}

#if defined(OS_OPTION_POWEROFF) || defined(OS_GDB_STUB)
#define OS_EXC_STOP_CORE_HWI_NUM OS_HWI_IPI_NO_02

#if defined(OS_GDB_STUB)
#include "prt_notifier.h"
void OsExcIrqHandler(void)
{
    OsNotifyDie(OS_EXC_STOP_CORE_HWI_NUM, NULL);
}
static int ExcNotifyDie(struct NotifierBlock *nb,
            int action, void *data)
{
    OsAsmIll(); /* 主动进入异常，最终会调用 OsCpuPowerOff */
    return NOTIFY_DONE;
}
static struct NotifierBlock g_excNotifier = {
    .call = ExcNotifyDie,
};

#else
void OsExcIrqHandler(void)
{
    U32 coreID = OsGetCoreID();
    g_isOtherCoreExc[coreID] = TRUE;
    OsAsmIll(); /* 主动进入异常，最终会调用 OsCpuPowerOff */
}
#endif
#endif

/*
 * 描述: EXC模块的初始化
 */
OS_SEC_L4_TEXT U32 OsExcConfigInit(void)
{
#if defined(OS_OPTION_CDA)
    OsCdaExcInit();
#endif

#if defined(OS_OPTION_SMP)
    U32 index;
    for(index = 0; index < OS_MAX_CORE_NUM; index++) {
        g_uniFlagAddr[index] = (uintptr_t)&g_runQueue[index].uniFlag;
    }
#endif

#if defined(OS_OPTION_POWEROFF) || defined(OS_GDB_STUB)
    U32 ret;
    ret = PRT_HwiSetAttr(OS_EXC_STOP_CORE_HWI_NUM, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_EXC_STOP_CORE_HWI_NUM, (HwiProcFunc)OsExcIrqHandler, 0);
    if (ret != OS_OK) {
        return ret;
    }

#if (OS_GIC_VER == 3)
    ret = PRT_HwiEnable(OS_EXC_STOP_CORE_HWI_NUM);
    if (ret != OS_OK) {
        return ret;
    }
#endif

#if defined(OS_GDB_STUB)
    OsRegisterDieNotifier(&g_excNotifier);
#endif
#endif

    return OS_OK;
}

/*
 * 描述: 切换到系统栈
 */
INIT_SEC_L4_TEXT uintptr_t OsSwitchToSysStack(uintptr_t sp)
{
    uintptr_t sysStackHigh;
    uintptr_t sysStackLow;
    uintptr_t dstSp;
    
    U32 core = THIS_CORE();
    sysStackHigh = OsGetSysStackEnd(core);
    sysStackLow  = OsGetSysStackStart(core);
    if (sp <= sysStackHigh && sp >= sysStackLow) {
        return sp;
    }

    RUNNING_TASK->stackPointer = (void *)sp;
    dstSp = sysStackHigh - sizeof(struct TskContext);

    if (memcpy_s((void *)dstSp, sizeof(struct TskContext), (void *)sp, sizeof(struct TskContext)) != EOK) {
        OS_GOTO_SYS_ERROR();
    }

    return dstSp;
}
