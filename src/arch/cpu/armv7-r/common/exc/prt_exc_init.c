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
#include "prt_coredump.h"

OS_SEC_BSS ExcTaskInfoFunc g_excTaskInfoGet;
extern uintptr_t __exc_stack_top;

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
 * 描述: 获取异常前的线程信息
 */
OS_SEC_ALW_INLINE INLINE void OsExcSetThreadInfo(struct ExcInfo *excInfo)
{
    U32 threadId = INVALIDPID;
    struct TskInfo taskInfo = {0};

    if (g_excTaskInfoGet != NULL) {
        g_excTaskInfoGet(&threadId, &taskInfo);
    }

    /* 记录发生异常时的线程ID，发生在任务和软中断中，此项具有意义，其他线程中，此项无意义 */
    excInfo->threadId = INVALIDPID;

    /* 设置异常前的线程类型 */
    if (OS_INT_COUNT > 0) {
        excInfo->threadType = EXC_IN_HWI;
    } else if ((UNI_FLAG & OS_FLG_TICK_ACTIVE) != 0) {
        excInfo->threadType = EXC_IN_TICK;
    } else if ((UNI_FLAG & OS_FLG_SYS_ACTIVE) != 0) {
        excInfo->threadType = EXC_IN_SYS;
    } else if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) != 0) {
        excInfo->threadType = EXC_IN_TASK;
        if (OsTskMaxNumGet() > 0) { /* 任务存在时 */
            excInfo->threadId = threadId;
        }
    } else { /* OS_FLG_BGD_ACTIVE没有置位，代表此时还在系统进程中，没有进入业务线程 */
        excInfo->threadType = EXC_IN_SYS_BOOT;
    }

    /* 任务栈栈底 */
    if (excInfo->threadType == EXC_IN_TASK) {
        excInfo->stackBottom = TRUNCATE((taskInfo.topOfStack + taskInfo.stackSize), OS_EXC_STACK_ALIGN);
    }
}
/*
 * 描述: 记录异常信息
 */
INIT_SEC_L4_TEXT void OsExcSaveInfo(struct ExcInfo *excInfo, struct ExcRegInfo *regs)
{
    U64 cycle;

    /* 记录异常嵌套计数 */
    excInfo->nestCnt = CUR_NEST_COUNT;

    /* 记录os版本号 */
    if (strncpy_s(excInfo->osVer, sizeof(excInfo->osVer), PRT_SysGetOsVersion(), (sizeof(excInfo->osVer) - 1)) != EOK) {
        OS_GOTO_SYS_ERROR();
    }
    excInfo->osVer[OS_SYS_OS_VER_LEN - 1] = '\0';

    /* 记录CPU ID */
    excInfo->coreId = 0x0U;

    /* 设置字节序 */
    /* 魔术字 */
    excInfo->byteOrder = OS_BYTE_ORDER;

    /* 记录CPU类型 */
    excInfo->cpuType = OsGetCpuType();

    /* 记录CPU TICK值 */
    cycle = OsCurCycleGet64();
    excInfo->cpuTick.cntHi = OS_GET_64BIT_HIGH_32BIT(cycle);
    excInfo->cpuTick.cntLo = (U32)cycle;

    /* 记录寄存器信息 */
    excInfo->regInfo = *regs;

    /* 记录异常前栈指针 */
    excInfo->sp = regs->SP;

    /* 记录异常前栈底,系统栈栈底 */
    excInfo->stackBottom = (uintptr_t)&__exc_stack_top;

    OsExcSetThreadInfo(excInfo);
}

/*
 * 描述: EXC模块的处理分发函数
 */
INIT_SEC_L4_TEXT void OsExcHandleEntry(U32 excType, struct ExcRegInfo *excRegs)
{
    struct ExcInfo *excInfo = OS_EXC_INFO_ADDR;

    UNI_FLAG |= (OS_FLG_HWI_ACTIVE | OS_FLG_EXC_ACTIVE);
    if (excType == OS_EXCEPT_FIQ) {
        OsExcFiqProc(excType);
        return;
    }

    /* 记录异常类型 */
    excInfo->excCause = excType;
    excInfo->fatalErrNo = OsFatalErrClr();

    CUR_NEST_COUNT++;
    
    if (CUR_NEST_COUNT == 1) {
        OsExcType(excType, excRegs);
        OsExcSysInfo(excType, excRegs);
        OsExcRegsInfo(excRegs);
    } else {
        OsCallStackInfo();
    }

    /* 记录异常信息 */
    OsExcSaveInfo(excInfo, excRegs);

#if defined(OS_OPTION_COREDUMP)
    coredump(excInfo);
#endif

    /* 回调异常钩子函数 */
    OsExcHookHandle();
}

/*
 * 描述: EXC模块的初始化
 */
OS_SEC_L4_TEXT U32 OsExcConfigInit(void)
{
    return OS_OK;
}
