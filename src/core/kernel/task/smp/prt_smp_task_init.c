/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-26
 * Description: Task schedule implementation
 */
#include "prt_task_internal.h"
#include "prt_irq_external.h"
#include "prt_smp_task_internal.h"

OS_SEC_L4_DATA struct TagListObject g_tskPercpuRecycleList[OS_MAX_CORE_NUM];

OS_SEC_L4_BSS volatile uintptr_t g_createTskLock;

/*
 * 描述：参与调度的核添加
 * 输入：coreId --- 核号
 * 返回：NA
 */
static OS_SEC_L4_TEXT void OsOnlineCoreAdd(U32 coreId)
{
    GET_RUNQ(coreId)->online = TRUE;
}

/*
 * 描述：task first time switch
 * 输入：none
 * 输出：none
 * 返回：none
 */
OS_SEC_L4_TEXT void OsFirstTimeSwitch(void)
{
    struct TagOsRunQue *runQue = THIS_RUNQ();

    RUNNING_TASK = OsPickNextTask(runQue); 

    OsOnlineCoreAdd(THIS_CORE());

    OsContextSwitch(OS_PST_ZOMBIE_TASK, RUNNING_TASK);

    return;
}

#if defined(OS_OPTION_TASK_DELETE)
/*
 * 描述：任务自删除回收中断处理函数
 * 输入：none
 * 输出：none
 * 返回：OS_OK on success or error code on failure
 */
OS_SEC_L4_TEXT U32 OsTskRecycleIsr(HwiArg arg)
{
    uintptr_t intSave;
    struct TagTskCb *taskCB = NULL;
    intSave = OsIntLock();
    struct TagListObject *recycleList = &g_tskPercpuRecycleList[THIS_CORE()];

    TSK_CREATE_DEL_LOCK();

    (void)arg;
    while (!ListEmpty(recycleList)) {
        taskCB = GET_TCB_PEND(OS_LIST_FIRST(recycleList));

        ListDelete(OS_LIST_FIRST(recycleList));
        ListAdd(&taskCB->pendList, &g_tskRecyleList);
    }
    TSK_CREATE_DEL_UNLOCK();
    OsIntRestore(intSave);
    return OS_OK;
}
/*
 * 描述：任务自删除回收中断
 * 输入：none
 * 输出：none
 * 返回：OS_OK on success or error code on failure
 */
OS_SEC_L4_TEXT U32 OsTskRecycleIPCInit(void)
{
    U32 ret;
    U32 loop;

    for (loop = 0; loop < g_maxNumOfCores; loop++) {
        OS_LIST_INIT(&g_tskPercpuRecycleList[loop]);
    }

    ret = PRT_HwiSetAttr(OS_SMP_MC_CORE_IPC_SGI, OS_SMP_MC_CORE_IPC_SGI_PRI, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    /* 创建一个核间中断 */
    ret = PRT_HwiCreate(OS_SMP_MC_CORE_IPC_SGI, (HwiProcFunc)OsTskRecycleIsr, 0);
    if (ret != OS_OK) {
        return ret;
    }
    
    /* 使能该核间中断 */
    (void)PRT_HwiEnable(OS_SMP_MC_CORE_IPC_SGI);

    return ret;
}
#endif

static OS_SEC_TEXT void OsSmpSchIsr(U32 irqNo)
{
    (void)irqNo;
}

static OS_SEC_TEXT U32 OsSmpSchIrqInit(void)
{
    U32 ret = PRT_HwiSetAttr(OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI,
                            OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI_PRI, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI, (HwiProcFunc)OsSmpSchIsr, 0);
    if (ret != OS_OK) {
        return ret;
    }

    (void)PRT_HwiEnable(OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI);

    return ret;
}

OS_SEC_ALW_INLINE INLINE U32 OsTskInitSmpTcb(void)
{
    U32 index;
    size_t size;

    g_threadNum += OS_MAX_TCB_NUM;

    /* Always reserve task idle for background. */
    size = OS_MAX_TCB_NUM *sizeof(struct TagTskCb);

    g_tskCbArray = (struct TagTskCb *)OsMemAllocAlign((U32)OS_MID_TSK, OS_MEM_DEFAULT_FSC_PT,
                                                    (U32)size, MEM_ADDR_ALIGN_016);
    if (g_tskCbArray == NULL) {
        return OS_ERRNO_TSK_NO_MEMORY;
    }
    
    /* connect all the TCBs in a doubly linked list. */
    if (memset_s(g_tskCbArray, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR();
    }

    for (index = 0; index < OS_MAX_TCB_NUM; index++) {
        g_tskCbArray[index].taskStatus = OS_TSK_UNUSED;
        g_tskCbArray[index].taskPid = index;
    }

    for (index = 0; index < g_tskMaxNum; index++) {
        ListTailAdd(&g_tskCbArray[index].pendList, &g_tskCbFreeList);
    }
    
    OsSpinLockInitInner(&g_createTskLock);
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsTskSmpDelaySysInit(void)
{
    U32 index;
    struct TagOsTskSortedDelayList *tskDlyBase = NULL;

    for(index = 0; index < g_maxNumOfCores; index++) {
        tskDlyBase = CPU_TSK_DELAY_BASE(index);
        OS_LIST_INIT(&tskDlyBase->tskList);
        tskDlyBase->nearestTicks = OS_TICKLESS_FOREVER;
        OsSpinLockInitInner(&tskDlyBase->spinLock);
    }
    return;
}
OS_SEC_ALW_INLINE INLINE U32 OsTskInitZombieTask(void)
{
    U32 index;
    struct TagTskCb *zombieTask = NULL;
    struct TagOsRunQue *runQue = NULL;

    zombieTask = OS_PST_ZOMBIE_TASK;
    zombieTask->scheClass = THIS_RUNQ()->schedClass;
    zombieTask->stackPointer = (void *)OsGetSysStackSP(THIS_CORE());

    zombieTask->taskStatus = OS_TSK_INUSE;
    zombieTask->priority = 0;

    for(index = 0; index < g_maxNumOfCores; index++) {
        runQue = GET_RUNQ(index);
        runQue->tskCurr = zombieTask;
        OsSpinLockInitInner(&runQue->spinLock);
    }
    return OS_OK;
}

/*
 * 描述：SMP Task init function
 * 输入：none
 * 输出：none
 * 返回：OS_OK on success or error code on failure
 */
OS_SEC_L4_TEXT U32 OsTskSMPInit(void)
{
    U32 ret;

    ret = OsTskInitSmpTcb();
    if (ret != OS_OK) {
        return ret;
    }

    ret = OsTskInitZombieTask();

#if defined(OS_OPTION_TASK_DELETE)
    ret = OsTskRecycleIPCInit();
    if(ret != OS_OK) {
        return ret;
    }
#endif
    OsTskSmpDelaySysInit();

    return OsSmpSchIrqInit();
}

/*
 * 描述：idle task create
 * 输入：none
 * 输出：none
 * 返回：OS_OK on success or error code on failure
 */
INIT_SEC_L4_TEXT U32 OsIdleTskSMPCreate(void)
{
    U32 ret;
    struct TagTskCb *taskCB = NULL;
    struct TskInitParam taskInitParam;
    char tskName[] = "IdleTskxxx";

    OsGetCoreStr((struct CoreNumStr *)&tskName[OS_CORE_STR_START_INDEX]);

    taskInitParam.taskEntry = (TskEntryFunc)g_tskIdleEntry;
    taskInitParam.stackSize = g_tskModInfo.idleStackSize;
    taskInitParam.name = tskName;
    taskInitParam.taskPrio = OS_TSK_PRIORITY_LOWEST;
    taskInitParam.stackAddr = PRT_MemAllocAlign((U32)OS_MID_TSK, OS_MEM_DEFAULT_FSC_PT,
        g_tskModInfo.idleStackSize, OS_TSK_STACK_SIZE_ALLOC_ALIGN);

    ret = OsTaskCreateOnly(&IDLE_TASK_ID, &taskInitParam, TRUE);
    if(ret != OS_OK) {
        return ret;
    }

    taskCB = GET_TCB_HANDLE(IDLE_TASK_ID);
    taskCB->scheClass = OS_SCHED_CLASS(taskCB->coreID);
    taskCB->nrCoresAllowed = 1;
    taskCB->coreAllowedMask = (OS_CORE_MASK)(1UL << (THIS_CORE()));

    TSK_STATUS_CLEAR(taskCB, OS_TSK_SUSPEND);
    OsTskReadyAdd(taskCB);

    return OS_OK;
}