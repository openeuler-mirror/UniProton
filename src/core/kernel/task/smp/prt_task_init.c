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
#include "prt_exc_external.h"
#include "prt_task_internal.h"
#include "prt_smp_task_internal.h"
#include "prt_signal_external.h"
#if defined(OS_OPTION_LOCALE)
#include "prt_posix_ext.h"
#endif
/* Unused TCBs and ECBs that can be allocated. */
OS_SEC_DATA struct TagListObject g_tskCbFreeList = LIST_OBJECT_INIT(g_tskCbFreeList);
OS_SEC_DATA struct TagListObject g_tskRecyleList = LIST_OBJECT_INIT(g_tskRecyleList);

/* tskmon队列 */
OS_SEC_BSS struct TagTskMonNode *g_tskMonList;

/* 任务栈的分区可配置 */
OS_SEC_L4_DATA U32 g_taskStackPtNp = OS_MEM_DEFAULT_FSC_PT;
/*
 * 描述：注册任务模块信息
 */
OS_SEC_L4_TEXT U32 OsTskRegister(struct TskModInfo *modInfo)
{
    if (((modInfo->maxNum) > (MAX_TASK_NUM)) || (modInfo->maxNum == 0)) {
        return OS_ERRNO_TSK_MAX_NUM_NOT_SUITED;
    }

    g_tskModInfo.maxNum = modInfo->maxNum;
    g_tskModInfo.defaultSize = modInfo->defaultSize;
    g_tskModInfo.idleStackSize = modInfo->idleStackSize;
    g_tskModInfo.magicWord = modInfo->magicWord;

    /* task模块有动态加载的场景 */
    if (g_tskIdleEntry == NULL) {
        g_tskIdleEntry = (TskEntryFunc)OsTskIdleBgd;
    }

    g_tskMaxNum = g_tskModInfo.maxNum;

    return OS_OK;
}

#if defined(OS_OPTION_TASK_INFO)
OS_SEC_L4_TEXT void OsTskInfoGet(TskHandle *threadId, struct TskInfo *taskInfo)
{
    U32 ret;

    if ((g_tskMaxNum > 0) && ((UNI_FLAG & OS_FLG_BGD_ACTIVE) != 0)) { /* 任务存在时 */
        ret = PRT_TaskSelf(threadId);
        if (ret == OS_OK) {
            /* 获取当前任务信息 */
            OS_ERR_RECORD(PRT_TaskGetInfo((*threadId), taskInfo));
        }
    }
}

OS_SEC_L4_TEXT U32 OsTaskNameAdd(U32 taskId, const char *name)
{
    struct TagTskCb *taskCb = GET_TCB_HANDLE(taskId);
    /* 不检测线程名同名 */
    if (strncpy_s(taskCb->name, sizeof(taskCb->name), name, OS_TSK_NAME_LEN - 1) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    return OS_OK;
}

OS_SEC_L4_TEXT void OsTaskNameGet(U32 taskId, char **taskName)
{
    struct TagTskCb *taskCb = GET_TCB_HANDLE(taskId);

    *taskName = taskCb->name;
}
#endif

/*
 * 描述：任务初始化
 */
OS_SEC_L4_TEXT U32  OsTskInit(void)
{
    U32 ret;
    ret = OsTskSMPInit();
    if (ret != OS_OK) {
        return ret;
    }

    g_taskScanHook = OsTaskScan;

#if defined(OS_OPTION_TASK_INFO)
    g_excTaskInfoGet = (ExcTaskInfoFunc)OsTskInfoGet;
    g_taskNameAdd = OsTaskNameAdd;
    g_taskNameGet = OsTaskNameGet;
#endif

#if defined(OS_OPTION_POWEROFF)
    g_sysPowerOffHook = OsCpuPowerOff;
#endif

    /* 设置任务切换钩子标志位 */
    UNI_FLAG |= OS_FLG_TSK_SWHK;
    return OS_OK;
}

/*
 * 描述：分配任务栈空间
 */
OS_SEC_L4_TEXT void *OsTskMemAlloc(U32 size)
{
    void *stackAddr = NULL;
    stackAddr = OsMemAllocAlign((U32)OS_MID_TSK, (U8)OS_MEM_DEFAULT_FSC_PT, size,
                                /* 内存已按16字节大小对齐 */
                                OS_TSK_STACK_SIZE_ALLOC_ALIGN);
    return stackAddr;
}
void SlaveTaskEntry()
{
    PRT_Printf("slave 1.\n");
    static U32 temp1 = 0;
    while (1) {
        PRT_TaskDelay(500);
        PRT_Printf("slave 1.\n");
        temp1++;
    }
}

void SlaveTaskEntry2()
{
    static U32 temp2 = 0;
    while (1) {
        PRT_Printf("slave 2.\n");
        PRT_TaskDelay(300);
        temp2++;
    }
}

U32 SlaveTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task 1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry;
    param.taskPrio = 25;
    param.name = "SlaveTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }
    
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry2;
    param.taskPrio = 30;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[1]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}
/*
 * 描述：激活任务管理
 */
OS_SEC_L4_TEXT U32 OsActivate(void)
{
    U32 ret;
    if(OsGetCoreID() != g_cfgPrimaryCore) {
        ret = SlaveTestInit();
        if (ret) {
            return ret;
        }
    }

    ret = OsIdleTskSMPCreate();

    if (ret != OS_OK) {
        return ret;
    }

    /* Indicate that background task is running. */
    UNI_FLAG |= OS_FLG_BGD_ACTIVE;

    if(OsGetHwThreadId() == g_primaryCoreId) {

        OS_MHOOK_ACTIVATE_PARA0(OS_HOOK_FIRST_TIME_SWH);

        OsSmpWakeUpSecondaryCore();
    }

    /* Start Multitasking. */
    OsFirstTimeSwitch();

    return OS_ERRNO_TSK_ACTIVE_FAILED;
}

/*
 * 描述：所有任务入口
 */
OS_SEC_L4_TEXT void OsTskEntry(TskHandle taskId)
{
    struct TagTskCb *taskCb;
    uintptr_t intSave;

    (void)taskId;

    taskCb = OsGetCurrentTcb();

    taskCb->taskEntry(taskCb->args[OS_TSK_PARA_0], taskCb->args[OS_TSK_PARA_1], taskCb->args[OS_TSK_PARA_2],
                      taskCb->args[OS_TSK_PARA_3]);

    intSave = OsIntLock();

    OS_TASK_LOCK_DATA = 0;

    /* PRT_TaskDelete不能关中断操作，否则可能会导致它核发SGI等待本核响应时死等 */
    OsIntRestore(intSave);

    OsTaskExit(taskCb);
}

/*
 * 描述：创建任务参数检查
 */
OS_SEC_L4_TEXT U32 OsTaskCreateParaCheck(const TskHandle *taskPid, struct TskInitParam *initParam)
{
    if ((taskPid == NULL) || (initParam == NULL)) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    if (initParam->taskEntry == NULL) {
        return OS_ERRNO_TSK_ENTRY_NULL;
    }

    if (initParam->stackSize == 0) {
        initParam->stackSize = g_tskModInfo.defaultSize;
    }

    if (((OS_TSK_STACK_SIZE_ALIGN - 1) & initParam->stackSize) != 0) {
        return OS_ERRNO_TSK_STKSZ_NOT_ALIGN;
    }

    if (((OS_TSK_STACK_SIZE_ALIGN - 1) & (uintptr_t)initParam->stackAddr) != 0) {
        return OS_ERRNO_TSK_STACKADDR_NOT_ALIGN;
    }

    if (initParam->stackSize < OS_TSK_MIN_STACK_SIZE) {
        return OS_ERRNO_TSK_STKSZ_TOO_SMALL;
    }
    /* 使用用户内存，则需要包含OS使用的资源，size最小值要包含OS的消耗 */
    if (initParam->stackAddr != 0) {
        if (OsCheckStackAddrOverflow(initParam->stackAddr, initParam->stackSize)) {
            return OS_ERRNO_TSK_STACKADDR_TOO_BIG;
        }
    }

    if (initParam->taskPrio > OS_TSK_PRIORITY_LOWEST) {
        return OS_ERRNO_TSK_PRIOR_ERROR;
    }

    /* 创建任务线程时做校验 */
    if (initParam->name == NULL || initParam->name[0] == '\0') {
        return OS_ERRNO_TSK_NAME_EMPTY;
    }

    return OS_OK;
}

OS_SEC_L4_TEXT void OsTskRecycle(void)
{
    struct TagTskCb *taskCb = NULL;

    /* 释放掉之前自删除任务的资源,自删除时,由于任务还处于运行态,不会及时释放资源 */
    /* 调用处已加recycle list锁 */
    while (!ListEmpty(&g_tskRecyleList)) {
        taskCb = GET_TCB_PEND(OS_LIST_FIRST(&g_tskRecyleList));
        ListDelete(OS_LIST_FIRST(&g_tskRecyleList));
        ListTailAdd(&taskCb->pendList, &g_tskCbFreeList);
        OsTskResRecycle(taskCb);
    }
}

OS_SEC_L4_TEXT void OsTskStackInit(U32 stackSize, uintptr_t topStack)
{
    U32 loop;
    U32 stackMagicWord = OS_TSK_STACK_MAGIC;

    /* 初始化任务栈，并写入栈魔术字 */
    for (loop = 1; loop < (stackSize / sizeof(U32)); loop++) {
        *((U32 *)topStack + loop) = stackMagicWord;
    }
    *((U32 *)(topStack)) = OS_TSK_STACK_TOP_MAGIC;
}

OS_SEC_L4_TEXT U32 OsTaskCreateRsrcInit(U32 taskId, struct TskInitParam *initParam, struct TagTskCb *taskCb,
                                                  uintptr_t **topStackOut, uintptr_t *curStackSize)
{
    U32 ret = OS_OK;
    uintptr_t *topStack = NULL;

    /* 创建任务线程 */
    if (g_taskNameAdd != NULL) {
        ret = g_taskNameAdd(taskId, initParam->name);
        if (ret != OS_OK) {
            return ret;
        }
    }

    /* 查看用户是否配置了任务栈，如没有，则进行内存申请，并标记为系统配置，如有，则标记为用户配置。 */
    if (initParam->stackAddr != 0) {
        topStack = (void *)(initParam->stackAddr);
        taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_USER;
        *curStackSize = initParam->stackSize;
    } else {
        topStack = OsTskMemAlloc(initParam->stackSize);
        if (topStack == NULL) {
            ret = OS_ERRNO_TSK_NO_MEMORY;
        } else {
            taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_SYS;
            *curStackSize = initParam->stackSize;
        }
    }
    if (ret != OS_OK) {
        return ret;
    }

    *topStackOut = topStack;
    return OS_OK;
}

OS_SEC_L4_TEXT void OsTskCreateTcbInit(uintptr_t stackPtr, struct TskInitParam *initParam,
    uintptr_t topStackAddr, uintptr_t curStackSize, struct TagTskCb *taskCb)
{
    /* Initialize the task's stack */
    taskCb->stackPointer = (void *)stackPtr;
    taskCb->args[OS_TSK_PARA_0] = (uintptr_t)initParam->args[OS_TSK_PARA_0];
    taskCb->args[OS_TSK_PARA_1] = (uintptr_t)initParam->args[OS_TSK_PARA_1];
    taskCb->args[OS_TSK_PARA_2] = (uintptr_t)initParam->args[OS_TSK_PARA_2];
    taskCb->args[OS_TSK_PARA_3] = (uintptr_t)initParam->args[OS_TSK_PARA_3];
    taskCb->topOfStack = topStackAddr;
    taskCb->stackSize = curStackSize;
    taskCb->taskSem = NULL;
    taskCb->priority = initParam->taskPrio;
    taskCb->taskEntry = initParam->taskEntry;
#if defined(OS_OPTION_EVENT)
    taskCb->event = 0;
    taskCb->eventMask = 0;
#endif
    taskCb->lastErr = 0;

    OS_LIST_INIT(&taskCb->semBList);
    OS_LIST_INIT(&taskCb->pendList);
    OS_LIST_INIT(&taskCb->timerList);

#if defined(OS_OPTION_POSIX)
    taskCb->tsdUsed = 0;
    taskCb->state = PTHREAD_CREATE_JOINABLE;
    taskCb->cancelState = PTHREAD_CANCEL_ENABLE;
    taskCb->cancelType = PTHREAD_CANCEL_DEFERRED;
    taskCb->cancelPending = 0;
    taskCb->cancelBuf = NULL;
    taskCb->retval = NULL;
    taskCb->joinCount = 0;
    taskCb->joinableSem = 0;
#if defined(OS_OPTION_POSIX_SIGNAL)
    taskCb->sigMask = 0;
    taskCb->sigWaitMask = 0;
    taskCb->sigPending = 0;
    taskCb->itimer = 0;
    INIT_LIST_OBJECT(&taskCb->sigInfoList);
    OsInitSigVectors(taskCb);
#endif
#if defined(OS_OPTION_LOCALE)
    taskCb->locale = (locale_t)libc_global_locale;
#endif
#endif
    OsTskSMPTcbInit(initParam, taskCb);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsTskCreateTcbStatusSet(struct TagTskCb *taskCB, const struct TskInitParam *initParam)
{
    (void)initParam;
    OsSpinLockTaskRq(taskCB);
    taskCB->taskStatus = OS_TSK_SUSPEND | OS_TSK_INUSE;
    OsSpinUnlockTaskRq(taskCB);
    return;
}

OS_SEC_ALW_INLINE INLINE void OsTaskManageFreeCb(struct TagTskCb *taskCB)
{
    TSK_CREATE_DEL_LOCK();

    ListAdd(&taskCB->pendList, &g_tskCbFreeList);

    TSK_CREATE_DEL_UNLOCK();
}
/*
 * 描述：创建一个任务但不进行激活
 */
OS_SEC_L4_TEXT U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam, bool isSmpIdle)
{
    U32 ret;
    U32 taskId;
    uintptr_t intSave;
    uintptr_t *topStack = NULL;
    void *stackPtr = NULL;
    struct TagTskCb *taskCb = NULL;
    U32 curStackSize = 0;

    ret = OsTaskCreateParaCheck(taskPid, initParam);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = OsIntLock();
    ret = OsTaskCreateChkAndGetTcb(&taskCb, isSmpIdle);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    taskId = taskCb->taskPid;
    ret = OsTaskCreateRsrcInit(taskId, initParam, taskCb, &topStack, &curStackSize);
    if (ret != OS_OK) {
        OsTaskManageFreeCb(taskCb);
        OsIntRestore(intSave);
        return ret;
    }

    OsTskStackInit(curStackSize, (uintptr_t)topStack);

    stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);

    OsTskCreateTcbInit((uintptr_t)stackPtr, initParam, (uintptr_t)topStack, curStackSize, taskCb);

    OsTskCreateTcbStatusSet(taskCb, initParam);

    *taskPid = taskId;
    OsIntRestore(intSave);
    OS_MHOOK_ACTIVATE_PARA1(OS_HOOK_TSK_CREATE, taskId);
    return OS_OK;
}

/*
 * 描述：创建一个任务但不进行激活
 */
OS_SEC_L4_TEXT U32 PRT_TaskCreate(TskHandle *taskPid, struct TskInitParam *initParam)
{
    return OsTaskCreateOnly(taskPid, initParam, FALSE);
}


#if defined(OS_OPTION_LOCALE)
OS_SEC_L4_TEXT locale_t *PRT_LocaleCurrent(void)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if (!tskCb) {
        return NULL;
    }

    return &tskCb->locale;
}
#endif