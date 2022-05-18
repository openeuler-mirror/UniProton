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
 * Description: 异常模块的C文件。
 */
#include "prt_clk.h"
#include "prt_exc_internal.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"
#include "prt_asm_cpu_external.h"

OS_SEC_BSS bool g_excSaveFlag;
OS_SEC_BSS struct ExcRegInfo g_excInfo;

// 异常时获取当前任务的信息
OS_SEC_BSS ExcTaskInfoFunc g_excTaskInfoGet;

OS_SEC_L4_TEXT U32 OsExcConfigInit(void)
{
    return OS_OK;
}

/*
 * 描述：获取异常线程信息
 */
static OS_SEC_L4_TEXT void OsExcGetThreadInfo(struct ExcInfo *excInfo)
{
    struct TskInfo taskInfo;
    U32 threadId = INVALIDPID;

    if (memset_s(&taskInfo, sizeof(struct TskInfo), 0, sizeof(struct TskInfo)) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }
#if defined(OS_OPTION_TASK_INFO)
    /* 任务存在时 */
    if ((OsTskMaxNumGet() > 0) && ((UNI_FLAG & OS_FLG_BGD_ACTIVE) != 0)) {
        /* 记录当前任务ID号 */
        if (PRT_TaskSelf(&threadId) == OS_OK) {
            /* 获取当前任务信息 */
            OS_ERR_RECORD(PRT_TaskGetInfo(threadId, &taskInfo));
        }
    }
#endif
    /* 记录发生异常时的线程ID，发生在任务中，此项具有意义，其他线程中，此项无意义 */
    excInfo->threadId = INVALIDPID;

    /* 设置异常前的线程类型 */
    if (g_intCount > CUR_NEST_COUNT) {
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
    } else {  // OS_FLG_BGD_ACTIVE没有置位，代表此时还在系统进程中，没有进入业务线程。
        excInfo->threadType = EXC_IN_SYS_BOOT;
    }

    /* MSP主堆栈 */
    if ((excInfo->sp >= OsGetSysStackTop()) && (excInfo->sp <= OsGetSysStackBottom())) {
        excInfo->stackBottom = OsGetSysStackBottom();
    } else { /* PSP进程堆栈 */
        excInfo->stackBottom = TRUNCATE((taskInfo.topOfStack + taskInfo.stackSize), OS_TSK_STACK_ADDR_ALIGN);
    }
}

/*
 * 描述：记录异常信息
 */
OS_SEC_L4_TEXT void OsExcSaveInfo(struct ExcRegInfo *regs)
{
    struct ExcInfo *excInfo = OS_EXC_INFO_ADDR;
    char *version = NULL;
    U64 count;

    /* 记录os版本号 */
    version = PRT_SysGetOsVersion();
    if (version != NULL) {
        if (strncpy_s(excInfo->osVer, sizeof(excInfo->osVer),
                      version, sizeof(excInfo->osVer) - 1) != EOK) {
            OS_GOTO_SYS_ERROR1();
        }
    }
    excInfo->osVer[OS_SYS_OS_VER_LEN - 1] = '\0';

    /* 记录异常类型 */
    excInfo->excCause = regs->excType;
    /* 记录CPU ID */
    excInfo->coreId = OsGetHwThreadId();

    /* 设置字节序 */
    excInfo->byteOrder = OS_BYTE_ORDER;

    /* 记录CPU类型 */
    excInfo->cpuType = OsGetCpuType();

    /* 记录CPU TICK值 */
    count = PRT_ClkGetCycleCount64();
    excInfo->cpuTick = count;

    /* 记录当前异常嵌套次数 */
    excInfo->nestCnt = CUR_NEST_COUNT;

    /* 记录寄存器信息 */
    excInfo->regInfo = (*regs);

    /* 记录异常前栈指针 */
    excInfo->sp = (regs->context)->sp;

    /* 记录异常前栈底 */
    excInfo->stackBottom = INVALIDSTACKBOTTOM;

    OsExcGetThreadInfo(excInfo);
}

static OS_SEC_L4_TEXT void OsExcRecordInfo(U32 excType, U32 faultAddr, struct ExcContext *excBufAddr)
{
    // 若为1时faultAddr有效
    U16 tmpFlag = (U16)OS_GET_32BIT_HIGH_16BIT(excType);

    g_excInfo.excType = excType;
    /* 此掩码在CDA解析时需要判断 */
    if (((U32)tmpFlag & OS_EXC_FLAG_FAULTADDR_VALID) != 0) {
        g_excInfo.faultAddr = faultAddr;
    } else {
        g_excInfo.faultAddr = OS_EXC_IMPRECISE_ACCESS_ADDR;
    }

    /* 异常上下文记录 */
    g_excInfo.context = excBufAddr;
}

static OS_SEC_L4_TEXT void OsReboot(void)
{
    while (1) {
        /* Wait for HWWDG to reboot board. */
    }
}
/*
 * 描述：EXC模块的处理分发函数
 */
OS_SEC_L4_TEXT void OsExcHandleEntryM4(U32 excType, U32 faultAddr, struct ExcContext *excBufAddr)
{
    CUR_NEST_COUNT++;
    g_intCount++;
    UNI_FLAG |= OS_FLG_HWI_ACTIVE;

    OsExcRecordInfo(excType, faultAddr, excBufAddr);

    if (OS_EXC_MAX_NEST_DEPTH < CUR_NEST_COUNT || g_excSaveFlag == TRUE) {
        OsReboot();
    }

    if (memset_s(OS_EXC_INFO_ADDR, EXC_RECORD_SIZE, 0, EXC_RECORD_SIZE) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    /* 没有初始化完成分配异常信息记录空间 */
    OsExcSaveInfo(&g_excInfo);
    g_excSaveFlag = TRUE;

    if (g_excModInfo.excepHook != NULL) {
        (void)g_excModInfo.excepHook(OS_EXC_INFO_ADDR);
    }
    OsReboot();
}
