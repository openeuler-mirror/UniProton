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
 * Description: 中断模块
 */
#include "prt_irq_internal.h"

/* 业务没有注册中断服务程序，却触发了中断时，记录的中断号 */
OS_SEC_L4_BSS U32 g_defHandlerHwiNum;

#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
OS_SEC_BSS struct TagHwiHandleForm *g_hwiForm;
#if defined(OS_OPTION_HWI_ATTRIBUTE)
OS_SEC_BSS struct TagHwiModeForm *g_hwiModeForm;
#endif

#else
/* 8字节对齐为了使用L64I指令 */
OS_SEC_BSS struct TagHwiHandleForm g_hwiForm[OS_HWI_FORMARRAY_NUM] __attribute__((aligned(8)));
#if defined(OS_OPTION_HWI_ATTRIBUTE)
OS_SEC_BSS struct TagHwiModeForm g_hwiModeForm[OS_HWI_MAX_NUM];
#endif

#endif

#if defined(OS_OPTION_HWI_COMBINE)
OS_SEC_BSS struct TagHwiCombineNode g_hwiCombineNode[OS_HWI_MAX_COMBINE_NODE];
OS_SEC_BSS struct TagHwiCombineNode *g_freeHwiComHead;
#endif

#if !defined(OS_OPTION_SMP)
OS_SEC_DATA U32 g_intCount = 0;
#endif
/*
 * 描述：硬中断默认注册钩子，会触发致命错误
 */
OS_SEC_L4_TEXT void OsHwiDefaultHandler(HwiArg arg)
{
    g_defHandlerHwiNum = OS_IRQ2HWI(arg);
    OS_REPORT_ERROR(OS_ERRNO_HWI_UNCREATED);
}

OS_SEC_ALW_INLINE INLINE void OsHwiDescFieldInit(U32 irqNum)
{
    OsHwiFuncSet(irqNum, OsHwiDefaultHandler);
    OsHwiParaSet(irqNum, irqNum);
}

/*
 * 描述：所有的中断描述符初始化
 */
OS_SEC_ALW_INLINE INLINE void OsHwiDescInitAll(void)
{
    U32 irqNum;
    for (irqNum = 0; irqNum < OS_HWI_MAX_NUM; irqNum++) {
        OsHwiDescFieldInit(irqNum);
    }
}

#if defined(OS_OPTION_HWI_COMBINE)
static OS_SEC_L4_TEXT void OsHwiCombineNodeInit(void)
{
    U32 idx;
    for (idx = 0; idx < OS_HWI_MAX_COMBINE_NODE - 1; idx++) {
        g_hwiCombineNode[idx].next = &g_hwiCombineNode[idx + 1];
        g_hwiCombineNode[idx].cmbHook = OsHwiDefaultHandler;
    }
    g_hwiCombineNode[idx].cmbHook = OsHwiDefaultHandler;
    g_hwiCombineNode[idx].next = NULL;

    /* 将所有未使用的组合型中断结点挂结到g_freeHwiComHead */
    g_freeHwiComHead = g_hwiCombineNode;
}
#endif

#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
static OS_SEC_L4_TEXT U32 OsHwiResourceAlloc(void)
{
    U32 size;
    uintptr_t addr;
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    size = g_hwiMaxNumConfig * (sizeof(struct TagHwiHandleForm) + sizeof(struct TagHwiModeForm));
#else
    size = g_hwiMaxNumConfig * sizeof(struct TagHwiHandleForm);
#endif
    /* 8字节对齐为了使用L64I指令 */
    addr = (uintptr_t)(uintptr_t *)OsMemAllocAlign(OS_MID_HWI, OS_MEM_DEFAULT_FSC_PT,
                                                   size, MEM_ADDR_ALIGN_008);
    if (addr == 0) {
        return OS_ERRNO_HWI_RESOURCE_ALLOC_FAILED;
    }
    if (memset_s((void *)addr, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    g_hwiForm = (struct TagHwiHandleForm *)addr;
    g_hwiModeForm = (struct TagHwiModeForm *)(addr + g_hwiMaxNumConfig * sizeof(struct TagHwiHandleForm));
#else
    g_hwiForm = (struct TagHwiHandleForm *)addr;
#endif
    return OS_OK;
}
#endif
/*
 * 描述：硬中断模块初始化
 */
OS_SEC_L4_TEXT U32 OsHwiConfigInit(void)
{
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
    U32 ret = OsHwiResourceAlloc();
    if (ret != OS_OK) {
        return ret;
    }
#endif
    OS_HWI_SPINLOCK_INIT();

    OsHwiDescInitAll();

#if defined(OS_OPTION_HWI_COMBINE)
    /* 组合型中断结点初始化 */
    OsHwiCombineNodeInit();
#endif

    /* 硬件中断控制器初始化 */
    OsHwiGICInit();

    return OS_OK;
}

OS_SEC_TEXT void OsHwiHookDispatcher(HwiHandle archHwi)
{
    /*
     * 将arch传入的参数转换成硬件中断号
     */
    HwiHandle hwiNum = OS_HWI_GET_HWINUM(archHwi);
    U32 irqNum = OS_HWI2IRQ(hwiNum);

    OS_MHOOK_ACTIVATE_PARA1(OS_HOOK_HWI_ENTRY, hwiNum);

    OsHwiHandleActive(irqNum);

    OS_MHOOK_ACTIVATE_PARA1(OS_HOOK_HWI_EXIT, hwiNum);
}

static OS_SEC_L4_TEXT U32 OsHwiSetAttrParaCheck(HwiHandle hwiNum, HwiPrior hwiPrio, HwiMode mode)
{
    if (OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (OsHwiPrioCheck(hwiPrio)) {
        return OS_ERRNO_HWI_PRI_ERROR;
    }

    if (OsHwiModeCheck(mode)) {
        return OS_ERRNO_HWI_MODE_ERROR;
    }
    return OS_OK;
}

static OS_SEC_L4_TEXT U32 OsHwiSetAttrConflictErrCheck(U32 irqNum, U32 hwiPrio, HwiMode mode)
{
    /* 如果该硬中断属性已经被设置, 则不允许变更设置,  除非先删除 */
    /* 支持私有服务程序的中断，各个核可独立配置，不做检查，其他的要做检查 */
    if (OS_HWI_MODE_INV(irqNum) || (OsHwiNeedPrivateIsr(irqNum) == TRUE)) {
        return OS_OK;
    }

    if (mode != OS_HWI_MODE_GET(irqNum)) {
        return OS_ERRNO_HWI_ATTR_CONFLICTED;
    }

    if (OsHwiPrioConflictCheck(irqNum, (U16)hwiPrio)) {
        return OS_ERRNO_HWI_ATTR_CONFLICTED;
    }
    return OS_OK;
}

/*
 * 描述：硬中断属性设置
 */
OS_SEC_L4_TEXT U32 PRT_HwiSetAttr(HwiHandle hwiNum, HwiPrior hwiPrio, HwiMode mode)
{
    uintptr_t intSave;
    U32 irqNum;
    U32 ret;

    ret = OsHwiSetAttrParaCheck(hwiNum, hwiPrio, mode);
    if (ret != OS_OK) {
        return ret;
    }

    irqNum = OS_HWI2IRQ(hwiNum);

    OS_HWI_IRQ_LOCK(intSave);

#if defined(OS_OPTION_HWI_ATTRIBUTE)
    ret = OsHwiSetAttrConflictErrCheck(irqNum, hwiPrio, mode);
    if (ret != OS_OK) {
        OS_HWI_IRQ_UNLOCK(intSave);
        return ret;
    }
#endif

#if (defined(OS_OPTION_HWI_PRIORITY) && ((OS_GIC_VER == 3) || defined(OS_PLIC_VER)))
    if (OsHwiPriorityGet(hwiNum) != hwiPrio) {
        OsHwiPrioritySet(hwiNum, hwiPrio);
    }
#endif

    OsHwiAttrSet(irqNum, hwiPrio, mode);

    OS_HWI_IRQ_UNLOCK(intSave);
    return OS_OK;
}

#if defined(OS_OPTION_HWI_COMBINE)
/*
 * 描述：遍历组合型硬中断模块信息
 */
OS_SEC_TEXT void OsHwiCombineDispatchHandler(HwiArg arg)
{
    struct TagHwiCombineNode *hook = NULL;

    hook = (struct TagHwiCombineNode *)arg;

    while (hook != NULL) {
        hook->cmbHook(hook->cmbParam);
        hook = hook->next;
    }
}

static OS_SEC_L4_TEXT U32 OsHwiCombineAlloc(U32 irqNum, HwiProcFunc handler, HwiArg arg)
{
    U8 freeNum;
    struct TagHwiCombineNode *newHook = NULL;
    struct TagHwiCombineNode *hwiCombineArray = NULL;
    struct TagHwiCombineNode *prev = NULL;

    /* 若组合型中断结点为空, 则再申请8个一组的组合型中断结点 */
    if (g_freeHwiComHead == NULL) {
        hwiCombineArray = OsMemAlloc(OS_MID_HWI, OS_MEM_DEFAULT_FSC_PT,
                                     (OS_HWI_MAX_COMBINE_NODE * sizeof(struct TagHwiCombineNode)));
        if (hwiCombineArray == NULL) {
            return OS_ERRNO_HWI_MEMORY_ALLOC_FAILED;
        }

        g_freeHwiComHead = hwiCombineArray;

        /* 组合型中断结点初始化 */
        for (freeNum = 0; freeNum < OS_HWI_MAX_COMBINE_NODE - 1; freeNum++) {
            hwiCombineArray[freeNum].next = &hwiCombineArray[freeNum + 1];
            hwiCombineArray[freeNum].cmbHook = OsHwiDefaultHandler;
        }

        hwiCombineArray[freeNum].cmbHook = OsHwiDefaultHandler;
        hwiCombineArray[freeNum].next = NULL;
    }

    /* 若组合型中断结点不为空, 则从前往后依次将未使用的组合型结点摘走 */
    newHook = g_freeHwiComHead;
    g_freeHwiComHead = newHook->next;

    /* 首次注册组合型中断服务函数 */
    if (OsHwiFuncGet(irqNum) == OsHwiDefaultHandler) {
        OsHwiFuncSet(irqNum, OsHwiCombineDispatchHandler);
        /* 组合型中断服务函数以当前摘取的结点作为入参, 通过该结点索引到下一个结点的中断服务函数 */
        OsHwiParaSet(irqNum, (uintptr_t)newHook);
    } else if (OsHwiFuncGet(irqNum) == OsHwiCombineDispatchHandler) {
        /* 取同一中断号的组合型结点表头, 遍历该组合型结点链表 */
        prev = (struct TagHwiCombineNode *)(HwiArg)OsHwiParaGet(irqNum);
        while (prev != NULL) {
            /* 若有出现相同中断号的用户中断钩子重复注册, 则报错返回 */
            if (handler == prev->cmbHook) {
                g_freeHwiComHead = newHook;
                return OS_ERRNO_HWI_COMBINEHOOK_ALREADY_CREATED;
            }

            if (prev->next == NULL) {
                prev->next = newHook;
                break;
            }
            prev = prev->next;
        }
    }

    newHook->next = NULL;
    newHook->cmbHook = handler;
    newHook->cmbParam = arg;

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsHwiCombineDelete(U32 irqNum)
{
    struct TagHwiCombineNode *hook = NULL;
    struct TagHwiCombineNode *newHook = NULL;

    hook = g_freeHwiComHead;
    newHook = (struct TagHwiCombineNode *)(HwiArg)OsHwiParaGet(irqNum);

    if (hook == NULL) {
        g_freeHwiComHead = newHook;
    } else {
        /* 索引至空闲链表表尾 */
        while (hook->next != NULL) {
            hook = hook->next;
        }

        /* 释放的结点插入到空闲链表表尾 */
        hook->next = newHook;
    }
    OsHwiFuncSet(irqNum, OsHwiDefaultHandler);
    OsHwiParaSet(irqNum, (uintptr_t)NULL);
}

#endif

static OS_SEC_L4_TEXT U32 OsHwiConnectHandle(U32 irqNum, HwiProcFunc handler, HwiArg arg)
{
#if defined(OS_OPTION_HWI_COMBINE)
    if (OS_HWI_MODE_GET(irqNum) == OS_HWI_MODE_COMBINE) {
        return OsHwiCombineAlloc(irqNum, handler, arg);
    }
#endif
    /* 判断是否允许注册 */
    if (OsHwiIsCanCreated(irqNum) == FALSE) {
        return OS_ERRNO_HWI_ALREADY_CREATED;
    }

    OsHwiFuncSet(irqNum, handler);
    OsHwiParaSet(irqNum, arg);

    OS_HWI_SET_HOOK_ATTR(OS_IRQ2HWI(irqNum), (U32)(OS_HWI_MODE_ATTR(irqNum)->prior), (uintptr_t)handler);

    return OS_OK;
}

/*
 * 描述：硬中断注册中断服务函数
 */
OS_SEC_L4_TEXT U32 PRT_HwiCreate(HwiHandle hwiNum, HwiProcFunc handler, HwiArg arg)
{
    uintptr_t intSave;
    U32 irqNum;
    U32 ret;

    if (OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (handler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    irqNum = OS_HWI2IRQ(hwiNum);

    OS_HWI_IRQ_LOCK(intSave);

#if defined(OS_OPTION_HWI_ATTRIBUTE)
    if (OS_HWI_MODE_INV(irqNum)) {
        OS_HWI_IRQ_UNLOCK(intSave);
        return OS_ERRNO_HWI_MODE_UNSET;
    }
#endif

    ret = OsHwiConnectHandle(irqNum, handler, arg);
    if (ret != OS_OK) {
        OS_HWI_IRQ_UNLOCK(intSave);
        return ret;
    }

    OS_HWI_IRQ_UNLOCK(intSave);
    return OS_OK;
}

static OS_SEC_L4_TEXT U32 OsHwiDeleteFormResume(U32 irqNum)
{
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    U32 ret = OsHwiAttrClear(irqNum);
    if (ret != OS_OK) {
        return ret;
    }
#endif

#if defined(OS_OPTION_HWI_COMBINE)
    if (OsHwiFuncGet(irqNum) == OsHwiCombineDispatchHandler) {
        OsHwiCombineDelete(irqNum);
        return OS_OK;
    }
#endif

    OsHwiFuncSet(irqNum, OsHwiDefaultHandler);
    /* 逻辑上保持默认服务程序的入参是逻辑中断号 */
    OsHwiParaSet(irqNum, irqNum);

    return OS_OK;
}

/*
 * 描述：硬中断删除函数
 */
OS_SEC_L4_TEXT U32 PRT_HwiDelete(HwiHandle hwiNum)
{
    U32 ret;
    uintptr_t intSave;
    U32 irqNum;

    if (OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    irqNum = OS_HWI2IRQ(hwiNum);

    /* 先disable中断 */
    (void)PRT_HwiDisable(hwiNum);

    OS_HWI_IRQ_LOCK(intSave);

    ret = OsHwiDeleteFormResume(irqNum);
    if (ret != OS_OK) {
        OS_HWI_IRQ_UNLOCK(intSave);
        return ret;
    }

    OS_HWI_IRQ_UNLOCK(intSave);
    return OS_OK;
}

OS_SEC_L4_TEXT void OsHwiDisableAll(void)
{
    U32 hwiNum;
    for (hwiNum = 0; hwiNum < OS_HWI_MAX_NUM; hwiNum++) {
        if (OS_HWI_NUM_CHECK(hwiNum)) {
            continue;
        }
        if (!OS_HWI_MODE_INV(hwiNum)) {
            PRT_HwiDisable(hwiNum);
        }
    }
}