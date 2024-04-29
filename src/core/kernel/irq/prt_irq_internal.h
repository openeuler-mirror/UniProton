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
 * Description: 中断模块内部头文件
 */
#ifndef PRT_IRQ_INTERNAL_H
#define PRT_IRQ_INTERNAL_H

#include "prt_irq_external.h"
#include "prt_hook_external.h"
#include "prt_mem_external.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "prt_buildef.h"
/*
 * 模块内宏定义
 */
#if defined(OS_OPTION_SMP)
extern volatile uintptr_t g_hwiLock;
#endif
#define OS_HWI_MODE_ATTR(irqNum) (&g_hwiModeForm[(irqNum)])
#define OS_HWI_MODE_GET(irqNum) (g_hwiModeForm[(irqNum)].mode)
#if defined(OS_OPTION_HWI_COMBINE)
#define OS_HWI_MODE_INV(irqNum) \
    ((OS_HWI_MODE_GET(irqNum) != OS_HWI_MODE_ENGROSS) && (OS_HWI_MODE_GET(irqNum) != OS_HWI_MODE_COMBINE))
#else
#define OS_HWI_MODE_INV(irqNum) (OS_HWI_MODE_GET(irqNum) != OS_HWI_MODE_ENGROSS)
#endif

#define OS_HWI_MODE_UNSET 0
#define OS_HWI_MAX_COMBINE_NODE 8

#if defined(OS_OPTION_SMP)
#define OS_HWI_SPL_LOCK()          \
    do {                           \
        OsSplLock(&g_hwiLock);     \
    } while (0)

#define OS_HWI_SPL_UNLOCK()        \
    do {                           \
        OsSplUnlock(&g_hwiLock);   \
    } while (0)

#define OS_HWI_IRQ_LOCK(intSave)   \
    do {                           \
        (intSave) = PRT_HwiLock(); \
        OsSplLock(&g_hwiLock);     \
    } while (0)

#define OS_HWI_IRQ_UNLOCK(intSave) \
    do {                           \
        OsSplUnlock(&g_hwiLock);   \
        PRT_HwiRestore(intSave);   \
    } while (0)

extern void OsHwiSplLock(void);
extern void OsHwiSplUnLock(void);

#define OS_HWI_SPINLOCK_INIT()                               \
    do {                                                     \
        OsSpinLockInitInner(&g_hwiLock);                     \
        OsHwiSetSplLockHook((OsVoidFunc)OsHwiSplLock);    \
        OsHwiSetSplUnlockHook((OsVoidFunc)OsHwiSplUnLock);\
    } while (0)
    
#else
#define OS_HWI_SPL_LOCK()
#define OS_HWI_SPL_UNLOCK()

#define OS_HWI_IRQ_LOCK(intSave)   \
    do {                           \
        (intSave) = PRT_HwiLock(); \
    } while (0)

#define OS_HWI_IRQ_UNLOCK(intSave) \
    do {                           \
        PRT_HwiRestore(intSave);   \
    } while (0)

#define OS_HWI_SPINLOCK_INIT() 
#endif

#if !defined(OS_OPTION_SMP)
struct TagHwiHandleForm {
    union {
        HwiProcFunc hook;               // 非私有中断时为正常hook
        struct TagHwiHandleForm *form;  // 私有中断时为指向私有form数组的指针
    };

#if defined(OS_OPTION_HWI_ATTRIBUTE)
    HwiArg param;
#endif
};

struct TagHwiModeForm {
    HwiMode mode;
    HwiPrior prior;
#if defined(OS_OPTION_HWI_AFFINITY)
    U32 affinityMask;
#endif
};
#endif

#if defined(OS_OPTION_HWI_COMBINE)
struct TagHwiCombineNode {
    HwiProcFunc cmbHook;
    HwiArg cmbParam;
    struct TagHwiCombineNode *next;
};
#endif

/*
 * 模块内全局变量声明
 */
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
extern struct TagHwiHandleForm *g_hwiForm;
extern struct TagHwiModeForm *g_hwiModeForm;
#else
extern struct TagHwiHandleForm g_hwiForm[OS_HWI_FORMARRAY_NUM];
extern struct TagHwiModeForm g_hwiModeForm[OS_HWI_MAX_NUM];
#endif

OS_SEC_ALW_INLINE INLINE struct TagHwiHandleForm *OsHwiHandleFormGet(U32 irqNum)
{
    return &g_hwiForm[irqNum];
}

OS_SEC_ALW_INLINE INLINE void OsHwiFuncSet(U32 irqNum, HwiProcFunc hook)
{
    struct TagHwiHandleForm *form = OsHwiHandleFormGet(irqNum);

    form->hook = hook;
}

OS_SEC_ALW_INLINE INLINE void OsHwiParaSet(U32 irqNum, HwiArg arg)
{
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    struct TagHwiHandleForm *form = OsHwiHandleFormGet(irqNum);

    form->param = arg;
#endif
    (void)irqNum;
    (void)arg;
}

OS_SEC_ALW_INLINE INLINE HwiProcFunc OsHwiFuncGet(U32 irqNum)
{
    struct TagHwiHandleForm *form = OsHwiHandleFormGet(irqNum);

    return form->hook;
}

OS_SEC_ALW_INLINE INLINE HwiArg OsHwiParaGet(U32 irqNum)
{
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    struct TagHwiHandleForm *form = OsHwiHandleFormGet(irqNum);

    return form->param;
#else
    return irqNum;
#endif
}

OS_SEC_ALW_INLINE INLINE void OsHwiHandleActive(U32 irqNum)
{
    HwiArg arg;
    struct TagHwiHandleForm *form = NULL;

    form = OsHwiHandleFormGet(irqNum);

#if defined(OS_OPTION_HWI_ATTRIBUTE)
    arg = form->param;
#else
    arg = irqNum;
#endif

    form->hook(arg);
}

#if defined(OS_OPTION_HWI_ATTRIBUTE)
OS_SEC_ALW_INLINE INLINE U32 OsHwiAttrClear(U32 irqNum)
{
    struct TagHwiModeForm *form = NULL;

    if (OS_HWI_MODE_INV(irqNum)) {
        return OS_ERRNO_HWI_DELETED;
    }

    /* 清除该中断号的工作模式 */
    form = OS_HWI_MODE_ATTR(irqNum);
    form->mode = OS_HWI_MODE_UNSET;

    return OS_OK;
}
#endif

OS_SEC_ALW_INLINE INLINE void OsHwiAttrSet(U32 irqNum, HwiPrior hwiPrio, HwiMode mode)
{
#if defined(OS_OPTION_HWI_ATTRIBUTE)
    struct TagHwiModeForm *form = OS_HWI_MODE_ATTR(irqNum);

    form->mode = mode;
    form->prior = hwiPrio;
#else
    (void)irqNum;
    (void)hwiPrio;
    (void)mode;
#endif
}

#if defined(OS_OPTION_HWI_COMBINE)
extern struct TagHwiCombineNode g_hwiCombineNode[OS_HWI_MAX_COMBINE_NODE];
extern struct TagHwiCombineNode *g_freeHwiComHead;
#endif

/*
 * 模块内内联函数定义
 */
#if defined(OS_OPTION_HWI_COMBINE)
OS_SEC_ALW_INLINE INLINE bool OsHwiModeCheck(HwiMode mode)
{
    if ((mode != OS_HWI_ATTR(OS_HWI_MODE_COMBINE, OS_HWI_TYPE_NORMAL)) &&
        (mode != OS_HWI_ATTR(OS_HWI_MODE_ENGROSS, OS_HWI_TYPE_NORMAL))) {
        return TRUE;
    }

    return FALSE;
}
#else
OS_SEC_ALW_INLINE INLINE bool OsHwiModeCheck(HwiMode mode)
{
    if ((mode != OS_HWI_ATTR(OS_HWI_MODE_ENGROSS, OS_HWI_TYPE_NORMAL))) {
        return TRUE;
    }

    return FALSE;
}
#endif

OS_SEC_ALW_INLINE INLINE bool OsHwiPrioCheck(HwiPrior hwiPrio)
{
#if defined(OS_OPTION_HWI_PRIORITY)
    return (bool)OS_HWI_PRIO_CHECK(hwiPrio);
#else
    (void)hwiPrio;
    return FALSE;
#endif
}

/* 如果不一致返回TRUE */
OS_SEC_ALW_INLINE INLINE bool OsHwiPrioConflictCheck(U32 irqNum, HwiPrior hwiPrio)
{
/* blong融合接口支持动态配置优先级 */
#if defined(OS_OPTION_HWI_PRIORITY)
    return (hwiPrio != OsHwiPriorityGet(OS_IRQ2HWI(irqNum)));
#else
    (void)irqNum;
    (void)hwiPrio;
    return FALSE;
#endif
}

OS_SEC_ALW_INLINE INLINE bool OsHwiNeedPrivateIsr(U32 irqNum)
{
    (void)irqNum;
    return FALSE;
}

/* 检查是否可以注册中断服务程序，TRUE表示允许 */
OS_SEC_ALW_INLINE INLINE bool OsHwiIsCanCreated(U32 irqNum)
{
    if (OsHwiFuncGet(irqNum) != OsHwiDefaultHandler) {
        return FALSE;
    }

    return TRUE;
}

#if defined(OS_OPTION_HWI_AFFINITY)
OS_SEC_ALW_INLINE INLINE void OsHwiAffinityMaskSet(U32 irqNum, U32 mask)
{
    struct TagHwiModeForm *from = OS_HWI_MODE_ATTR(irqNum);
    from->affinityMask = mask;
}
#endif
#endif /* PRT_IRQ_INTERNAL_H */
