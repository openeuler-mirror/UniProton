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
 * Description: UniProton的初始化C文件。
 */
#include "prt_config_internal.h"
#include "cpu_config.h"
#include "prt_cpu_external.h"
#include "prt_log.h"

#if (defined(OS_OPTION_SMP) || defined(OS_ARMV8))
RESET_SEC_DATA U32 g_cfgPrimaryCore = OS_SYS_CORE_PRIMARY;
#endif
#if defined(OS_OPTION_SMP)
extern U32 g_slaveTickEnable;
#endif

#if defined(LOG_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

OS_SEC_ALW_INLINE INLINE void OsConfigAddrSizeGet(uintptr_t addr, uintptr_t size,
                                                  uintptr_t *destAddr, uintptr_t *destSize)
{
    *destAddr = addr;
    *destSize = size;
}

U32 OsMemConfigReg(void)
{
    U32 ret;

    ret = OsFscMemInit(OS_MEM_FSC_PT_ADDR, OS_MEM_FSC_PT_SIZE);
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

U32 OsMemDefPtInit(void)
{
    return OS_OK;
}

U32 OsMemConfigInit(void)
{
    /* 系统默认FSC内存分区初始化 */
    return OsMemDefPtInit();
}

#if defined(OS_ARCH_ARMV8)
U64 GetGenericTimerFreq(void);
#endif

U32 OsSystemReg(void)
{
    struct SysModInfo sysModInfo;

#if defined(OS_ARCH_ARMV8)
    sysModInfo.systemClock = (U32)GetGenericTimerFreq();
#else
    sysModInfo.systemClock = OS_SYS_CLOCK;
#endif
    sysModInfo.cpuType = OS_CPU_TYPE;
    sysModInfo.sysTimeHook = OS_SYS_TIME_HOOK;
#if defined(OS_OPTION_SMP)
    sysModInfo.coreRunNum = OS_SYS_CORE_RUN_NUM;
    sysModInfo.coreMaxNum = OS_SYS_CORE_MAX_NUM;
    sysModInfo.corePrimary = OS_SYS_CORE_PRIMARY;
#endif
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
    sysModInfo.hwiMaxNum = OS_HWI_MAX_NUM_CONFIG;
#endif

    return OsSysRegister(&sysModInfo);
}

#if (OS_INCLUDE_SEM == YES)
U32 OsSemConfigReg(void)
{
    struct SemModInfo semModInfo;

    semModInfo.maxNum = OS_SEM_MAX_SUPPORT_NUM;
    return OsSemRegister(&semModInfo);

}
#endif

#if (OS_INCLUDE_TASK == YES)
void OsTaskInfoSet(struct TskModInfo *taskModInfo)
{
    taskModInfo->maxNum = OS_TSK_MAX_SUPPORT_NUM;
    taskModInfo->defaultSize = OS_TSK_DEFAULT_STACK_SIZE;
    taskModInfo->idleStackSize = OS_TSK_IDLE_STACK_SIZE;
    taskModInfo->magicWord = WORD_PACK((U32)OS_TSK_STACK_MAGIC_WORD);
}
#else
#error "OS_INCLUDE_TASK MUST BE YES! The SWI has been cut out，the task can not cut out."
#endif

U32 OsHookConfigReg(void)
{
    struct HookModInfo hookModInfo;

    hookModInfo.maxNum[OS_HOOK_HWI_ENTRY] = OS_HOOK_HWI_ENTRY_NUM;
    hookModInfo.maxNum[OS_HOOK_HWI_EXIT] = OS_HOOK_HWI_EXIT_NUM;
    hookModInfo.maxNum[OS_HOOK_TSK_CREATE] = OS_HOOK_TSK_CREATE_NUM;
    hookModInfo.maxNum[OS_HOOK_TSK_DELETE] = OS_HOOK_TSK_DELETE_NUM;
    hookModInfo.maxNum[OS_HOOK_TSK_SWITCH] = OS_HOOK_TSK_SWITCH_NUM;
    hookModInfo.maxNum[OS_HOOK_IDLE_PERIOD] = OS_HOOK_IDLE_NUM;

    return OsHookRegister(&hookModInfo);
}

U32 OsSysConfigReg(void)
{
    U32 ret;

    ret = OsSystemReg();
    if (ret != OS_OK) {
        return ret;
    }
    return OS_OK;
}

#if (OS_INCLUDE_TICK == YES)
U32 OsTickConfigReg(void)
{
    struct TickModInfo tickModInfo;

    tickModInfo.tickPerSecond = OS_TICK_PER_SECOND;
    tickModInfo.tickPriority = 0;

    return OsTickRegister(&tickModInfo);
}
#endif

#if (OS_INCLUDE_TASK == YES)
U32 OsTskConfigReg(void)
{
    struct TskModInfo taskModInfo;
    OsTaskInfoSet(&taskModInfo);
    return OsTskRegister(&taskModInfo);
}
#endif

#if (OS_INCLUDE_QUEUE == YES)
U32 OsQueueConfigReg(void)
{
    return OsQueueRegister(OS_QUEUE_MAX_SUPPORT_NUM);
}
#endif

#if (OS_INCLUDE_CPUP == YES)
U32 OsCpupConfigReg(void)
{
    struct CpupModInfo cpupModInfo;

    cpupModInfo.cpupWarnFlag = (bool)OS_CONFIG_CPUP_WARN;
    cpupModInfo.sampleTime = OS_CPUP_SAMPLE_INTERVAL;
    cpupModInfo.warn = OS_CPUP_SHORT_WARN;
    cpupModInfo.resume = OS_CPUP_SHORT_RESUME;

    return OsCpupRegister(&cpupModInfo);
}
#endif

#if (OS_INCLUDE_TICK_SWTMER == YES)
U32 OsSwTmrConfigInit(void)
{
    return OsSwTmrInit(OS_TICK_SWITIMER_MAX_NUM);
}
#endif

#if (OS_INCLUDE_CPUP == YES)
#if (OS_INCLUDE_TICK == NO)
#error "OS_INCLUDE_CPUP depend on OS_INCLUDE_TICK!"
#endif
U32 OsCpupConfigInit(void)
{
    U32 ret;

    ret = OsCpupInit();
    if (ret != OS_OK) {
        return ret;
    }

    if (OS_CONFIG_CPUP_WARN == YES) {
#if defined(OS_OPTION_CPUP_WARN)
        OsCpupWarnInit();
#else
        return OS_ERRNO_SYS_NO_CPUP_WARN;
#endif
    }
    return OS_OK;
}
#endif

#if (OS_INCLUDE_SEM == YES)
U32 OsSemConfigInit(void)
{
    return OsSemInit();
}
#endif

#if (OS_INCLUDE_TASK == YES)
U32 OsTskConfigInit(void)
{
    return OsTskInit();
}
#endif

#if defined(OS_OPTION_SMP)
INIT_SEC_L4_TEXT U32 OsSchedRunQueConfigInit(void)
{
    OsSchedRunQueInit();
    return OS_OK;
}
#endif

static U32 OsHwiConfigReg(void)
{
#if (OS_INCLUDE_GIC_BASE_ADDR_CONFIG == YES)
    U32 ret;
    ret = OsGicConfigRegister((uintptr_t)OS_GIC_BASE_ADDR, (uintptr_t)OS_GICR_OFFSET, (uintptr_t)OS_GICR_STRIDE);
    if (ret != OS_OK) {
        return ret;
    }
#endif
    return OS_OK;
}

#if defined(OS_OPTION_LOG)
static U32 PRT_LogMemInit(void)
{
    PRT_LogInit(MMU_LOG_MEM_ADDR);
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "PRT_Log init success1", 21);
    return OS_OK;
}
#endif

/* 系统初始化注册表 */
struct OsModuleConfigInfo g_moduleConfigTab[] = {
    /* {模块号， 模块注册函数， 模块初始化函数} */
#if defined(OS_OPTION_LOG)
    {OS_MID_LOG, {NULL, PRT_LogMemInit}},
#endif
    {OS_MID_SYS, {OsSysConfigReg, NULL}},
    {OS_MID_MEM, {OsMemConfigReg, OsMemConfigInit}},
    {OS_MID_HWI, {OsHwiConfigReg, OsHwiConfigInit}},
    {OS_MID_HARDDRV, {NULL, PRT_HardDrvInit}},
    {OS_MID_HOOK, {OsHookConfigReg, OsHookConfigInit}},
    {OS_MID_EXC, {NULL, OsExcConfigInit}},
#if defined(OS_OPTION_SMP)
    {OS_MID_SCHED, {NULL, OsSchedRunQueConfigInit}},
#endif
#if (OS_INCLUDE_TASK == YES)
    {OS_MID_TSK, {OsTskConfigReg, OsTskConfigInit}},
#endif
#if (OS_INCLUDE_TICK == YES)
    {OS_MID_TICK, {OsTickConfigReg, OsTickConfigInit}},
#endif

#if (OS_INCLUDE_TICK_SWTMER == YES)
    {OS_MID_SWTMR, {NULL, OsSwTmrConfigInit}},
#endif

#if (OS_INCLUDE_CPUP == YES)
    {OS_MID_CPUP, {OsCpupConfigReg, OsCpupConfigInit}},
#endif
#if (OS_INCLUDE_SEM == YES)
    {OS_MID_SEM, {OsSemConfigReg, OsSemConfigInit}},
#endif
#if (OS_INCLUDE_QUEUE == YES)
    {OS_MID_QUEUE, {OsQueueConfigReg, OsQueueConfigInit}},
#endif
    {OS_MID_APP, {NULL, PRT_AppInit}},

    {OS_MID_BUTT, {NULL, NULL}},
};

/*
 * 描述：OS模块注册、初始化运行函数
 */
U32 OsModuleConfigRun(enum OsinitPhaseId initPhaseId, U32 initPhase)
{
    U32 idx = 0;
    U32 ret = OS_OK;
    while (g_moduleConfigTab[idx].moudleId != OS_MID_BUTT) {
        if (g_moduleConfigTab[idx].moudleConfigFunc[initPhaseId] == NULL) {
            idx++;
            continue;
        }
        ret = g_moduleConfigTab[idx].moudleConfigFunc[initPhaseId]();
        if (ret != OS_OK) {
            break;
        }
        idx++;
    }
    return ret;
}
U32 OsRegister(void)
{
    return OsModuleConfigRun(OS_REGISTER_ID, OS_REGISTER_PHASE);
}

/*
 * 描述：OsInitialize阶段
 */
U32 OsInitialize(void)
{
    return OsModuleConfigRun(OS_INIT_ID, OS_INITIALIZE_PHASE);
}

/*
 * 描述：OsStart阶段
 */
U32 OsStart(void)
{
    U32 ret;
#if (OS_INCLUDE_TICK == YES)
    /* 表示系统在进行启动阶段，匹配MOUDLE_ID之后，标记进入TICK模块的启动 */
    ret = OsTickStart();
    if (ret != OS_OK) {
        return ret;
    }
#endif
#if (OS_INCLUDE_TASK == YES)
    /* 表示系统在进行启动阶段，匹配MOUDLE_ID之后，标记进入任务模块的启动 */
#if defined(LOG_TESTCASE)
    if(OsGetCoreID() != g_cfgPrimaryCore) {
        (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "slv init enter", 14);
        Init(0, 0, 0, 0);
    }
#endif
    ret = OsActivate();
#else
    ret = OS_OK;
#endif

    return ret;
}

#if defined(OS_OPTION_SMP)
INIT_SEC_L4_TEXT U32 OsSlaveSchedTriggerOtherCoreInit(void)
{
    U32 ret;
    ret = PRT_HwiEnable(OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI);
    if (ret != OS_OK) {
        return ret;
    }
    return ret;
}

INIT_SEC_L4_TEXT U32 OsSlaveTskRecycleIPCInit(void)
{
    U32 ret;
    ret = PRT_HwiEnable(OS_SMP_MC_CORE_IPC_SGI);
    if (ret != OS_OK) {
        return ret;
    }
    return ret;
}

#if defined(OS_OPTION_POWEROFF)
INIT_SEC_L4_TEXT U32 OsStopOtherCoreInit(void)
{
    U32 ret;
    ret = PRT_HwiEnable(OS_SMP_EXC_STOP_OTHER_CORE_SGI);
    if (ret != OS_OK) {
        return ret;
    }
    return ret;
}
#endif

INIT_SEC_L4_TEXT U32 OsTickTriggerOtherCoreInit(void)
{
    U32 ret;
    ret = PRT_HwiEnable(OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI);
    if (ret != OS_OK) {
        return ret;
    }
    if (OsGetCoreID() == (OS_SYS_CORE_PRIMARY + OS_SYS_CORE_RUN_NUM - 1)) {
        g_slaveTickEnable = TRUE;
    }
    return ret;
}

INIT_SEC_L4_TEXT U32 OsSmpPreInit(void)
{
    U32 ret;
    if(OsGetCoreID() != OS_SYS_CORE_PRIMARY) {
        ret = OsModuleInit();
        if(ret != OS_OK) {
            return ret;
        }

        ret = OsSlaveSchedTriggerOtherCoreInit();
        if (ret != OS_OK) {
            return ret;
        }

        ret = OsSlaveTskRecycleIPCInit();
        if (ret != OS_OK) {
            return ret;
        }

#if defined(OS_OPTION_TICKLESS)
        ret = OsTickTriggerOtherCoreInit();
        if (ret != OS_OK) {
            return ret;
        }
#else
        ret = OsCoreTimerSecondaryInit();
        if (ret != OS_OK) {
            return ret;
        }
#endif

#if defined(OS_OPTION_POWEROFF)
        ret = OsStopOtherCoreInit();
        if (ret != OS_OK) {
            return ret;
        }
#endif

        ret = OsStart();
        if(ret != OS_OK) {
            return ret;
        }

        /* 正常情况执行不应该到达这 */
        return OS_FAIL;
    }
    return OS_OK;

}
#endif

S32 OsConfigStart(void)
{
    U32 ret;

#if defined(OS_OPTION_SMP)
    ret = OsSmpPreInit();
    if(ret != OS_OK) {
        return (S32)ret;
    }
#endif

    OsHwInit();

    /* OS模块注册 */
    ret = OsRegister();
    if (ret != OS_OK) {
        return (S32)ret;
    }
    
    /* OS模块初始化 */
    ret = OsInitialize();
    if (ret != OS_OK) {
        return (S32)ret;
    }

    /* OS启动调度 */
    ret = OsStart();
    if (ret != OS_OK) {
        return (S32)ret;
    }

    /* Execution should not reach this point */
    return (S32)OS_ERROR;
}
