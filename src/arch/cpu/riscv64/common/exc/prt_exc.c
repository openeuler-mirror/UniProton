/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-02-22
 * Description: riscv内部异常模块函数
 */
#include "prt_exc_internal.h"
#include "prt_attr_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_exc_external.h"
#include "prt_irq_external.h"
#include "prt_task_external.h"
#include "prt_hook_external.h"
#include "prt_mem_external.h"
#include "prt_sem_external.h"
#include "prt_task_external.h"
#include "prt_queue_external.h"
#include "prt_swtmr_external.h"
#include "prt_timer_external.h"

// 异常时获取当前任务的信息
OS_SEC_BSS ExcTaskInfoFunc g_excTaskInfoGet;

// 异常callee 寄存器记录
OS_SEC_BSS struct ExcCalleeInfo g_excCalleeInfo;

// 异常发生时刻前 运行的任务信息
OS_SEC_BSS struct TskInfo taskInfo;

// 异常cause  寄存器记录
OS_SEC_BSS struct ExcCauseRegInfo g_causeRegInfo;

/*
 * 描述: 获取异常前的线程信息
 */
OS_SEC_ALW_INLINE INLINE void OsExcSetThreadInfo(struct ExcInfo *excInfo)
{
    U32 threadId = INVALIDPID;

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
    
    if(g_excTaskInfoGet != NULL) {
        excInfo->task = &taskInfo;
    }
    else {
        excInfo->task = NULL;
    }

    /* 任务栈栈底 */
    if (excInfo->threadType == EXC_IN_TASK) {
        excInfo->stackBottom = TRUNCATE((taskInfo.topOfStack + taskInfo.stackSize), OS_EXC_STACK_ALIGN);
    }
}

INIT_SEC_L4_TEXT void OsExcSaveInfo(struct ExcInfo *excInfo, struct ExcCalleeInfo *calleeInfo, struct ExcCauseRegInfo* regInfo)
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
    excInfo->cpuTick = cycle;

    /* 记录异常原因寄存器信息 */
    excInfo->excCause = *regInfo;

    /* 记录callee寄存器信息 */
    excInfo->excContext = *calleeInfo;

    /* 记录异常前栈底,系统栈栈底 */
    excInfo->stackBottom = (uintptr_t)&__os_sys_sp_end;
    
    OsExcSetThreadInfo(excInfo);
}

INLINE void OsReboot(void)
{
    while (1) {
        /* Wait for HWWDG to reboot board. */
        OS_EMBED_ASM("wfi");
    }
}

/*
 * 描述：EXC模块的处理分发函数
 */
OS_SEC_L4_TEXT void OsExcHandleEntryRISCV()
{
    CUR_NEST_COUNT++;
    UNI_FLAG |= (OS_FLG_EXC_ACTIVE);
    
    if (CUR_NEST_COUNT > OS_EXC_MAX_NEST_DEPTH) {
        OsReboot();
    }

    if (g_excModInfo.excepHook != NULL) {
        (void)g_excModInfo.excepHook(OS_EXC_INFO_ADDR);
    }
    OsReboot();
}

OS_SEC_L4_TEXT U32 OsExcConfigInit(void)
{
    return OS_OK;
}