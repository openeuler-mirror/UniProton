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
 * Description: HOOK模块的内部头文件
 */
#ifndef PRT_HOOK_EXTERNAL_H
#define PRT_HOOK_EXTERNAL_H

#include "prt_hook.h"
#include "prt_sys.h"

/* 限制：参数类型不能为U64大于uintptr_t */
typedef void (*OsFunPara0)(void);
typedef void (*OsFunPara1)(uintptr_t);
typedef void (*OsFunPara2)(uintptr_t, uintptr_t);
typedef void (*OsFunPara3)(uintptr_t, uintptr_t, uintptr_t);
typedef void (*OsFunPara4)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
typedef void (*OsFunPara5)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

/*
 * 内核钩子管理原则
 * 1）服务于内核模块的钩子才可纳入钩子模块统一管理，例如补丁、shell模块的钩子就不适合。
 * 内核模块指mem,kernel,ipc,服务于内核的arch。
 * 2）没有返回值，没有输出参数的钩子才可纳入钩子模块统一管理。
 */
#define OS_MHOOK_BOUNDARY ((U32)1)

#define OS_MHOOK_NODE_DEAD ((OsVoidFunc)2)

// 除了OS_HOOK_EMPTY和OS_MHOOK_BOUNDARY外，都是有效钩子
#define OS_MHOOK_IS_VALID(hook) ((uintptr_t)(hook) > OS_MHOOK_BOUNDARY)

#define OS_MHOOK_NOT_BOUNDARY(hook) ((uintptr_t)(hook) != OS_MHOOK_BOUNDARY)

OS_SEC_ALW_INLINE INLINE bool OsMhookBoundaryCheck(OsVoidFunc hook)
{
    return ((uintptr_t)hook <= OS_MHOOK_BOUNDARY);
}

OS_SEC_ALW_INLINE INLINE bool OsMhookValidCheck(OsVoidFunc hook)
{
    return (hook != OS_MHOOK_NODE_DEAD);
}

#define OS_MHOOK_ACTIVATE_PROC(hook, funcType, pfn, list)                        \
    do {                                                                         \
        OsVoidFunc *tmp_ = hook;                                                 \
        while (!OsMhookBoundaryCheck((OsVoidFunc)((pfn) = (funcType)(*tmp_)))) { \
            if (OsMhookValidCheck((OsVoidFunc)(pfn))) {                          \
                (list);                                                          \
            }                                                                    \
            tmp_++;                                                              \
        }                                                                        \
    } while (0)

#define OS_MHOOK_ACTIVATE(hookType, funcType, list)              \
    do {                                                         \
        OsVoidFunc *hook = g_hookCb[(hookType)].mulHook;         \
        funcType pfn;                                            \
        if (hook != NULL) {                                      \
            OS_MHOOK_ACTIVATE_PROC(hook, funcType, pfn, (list)); \
        }                                                        \
    } while (0)

#define OS_SHOOK_ACTIVATE(hookType, funcType, list)            \
    do {                                                       \
        funcType pfn = (funcType)g_hookCb[(hookType)].sigHook; \
        if (pfn != NULL)                                       \
            (list);                                            \
    } while (0)

#define OS_MHOOK_ACTIVATE_PARA0(hookType) OS_MHOOK_ACTIVATE((hookType), OsFunPara0, pfn())
#define OS_MHOOK_ACTIVATE_PARA1(hookType, arg0) OS_MHOOK_ACTIVATE((hookType), OsFunPara1, pfn((uintptr_t)(arg0)))
#define OS_MHOOK_ACTIVATE_PARA2(hookType, arg0, arg1) OS_MHOOK_ACTIVATE((hookType), \
    OsFunPara2, pfn((uintptr_t)(arg0), (uintptr_t)(arg1)))
#define OS_MHOOK_ACTIVATE_PARA3(hookType, arg0, arg1, arg2) OS_MHOOK_ACTIVATE((hookType), \
    OsFunPara3, pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2)))
#define OS_MHOOK_ACTIVATE_PARA4(hookType, arg0, arg1, arg2, arg3) \
    OS_MHOOK_ACTIVATE((hookType),                                 \
    OsFunPara4, pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3)))
#define OS_MHOOK_ACTIVATE_PARA5(hookType, arg0, arg1, arg2, arg3, arg4) \
    OS_MHOOK_ACTIVATE((hookType), OsFunPara5,                           \
    pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4)))

#define OS_SHOOK_ACTIVATE_PARA0(hookType) OS_SHOOK_ACTIVATE((hookType), OsFunPara0, pfn())
#define OS_SHOOK_ACTIVATE_PARA1(hookType, arg0) OS_SHOOK_ACTIVATE((hookType), OsFunPara1, pfn((uintptr_t)(arg0)))
#define OS_SHOOK_ACTIVATE_PARA2(hookType, arg0, arg1) OS_SHOOK_ACTIVATE((hookType), \
    OsFunPara2, pfn((uintptr_t)(arg0), (uintptr_t)(arg1)))
#define OS_SHOOK_ACTIVATE_PARA3(hookType, arg0, arg1, arg2) OS_SHOOK_ACTIVATE((hookType), \
    OsFunPara3, pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2)))
#define OS_SHOOK_ACTIVATE_PARA4(hookType, arg0, arg1, arg2, arg3) \
    OS_SHOOK_ACTIVATE((hookType), OsFunPara4,                     \
    pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3)))
#define OS_SHOOK_ACTIVATE_PARA5(hookType, arg0, arg1, arg2, arg3, arg4) \
    OS_SHOOK_ACTIVATE((hookType), OsFunPara5,                           \
    pfn((uintptr_t)(arg0), (uintptr_t)(arg1), (uintptr_t)(arg2), (uintptr_t)(arg3), (uintptr_t)(arg4)))

#define HOOK_ADD_IRQ_LOCK(intSave) \
    do {                           \
        (intSave) = PRT_HwiLock(); \
    } while (0)
#define HOOK_ADD_IRQ_UNLOCK(intSave) PRT_HwiRestore((intSave))

#define HOOK_DEL_IRQ_LOCK(intSave) \
    do {                           \
        (intSave) = PRT_HwiLock(); \
    } while (0)
#define HOOK_DEL_IRQ_UNLOCK(intSave) PRT_HwiRestore((intSave))

enum {
    OS_HOOK_TICK_ENTRY = OS_HOOK_TYPE_NUM,
    OS_HOOK_TICK_EXIT,
    OS_HOOK_FIRST_TIME_SWH,
    OS_HOOK_TSK_MON, /* TSKMON钩子 */
    OS_HOOK_CPUP_WARN, /* CPUP告警钩子 */
    OS_HOOK_ERR_REG, /* 错误处理钩子 */
    OS_HOOK_IDLE_PREFIX, /* IDLE前置钩子 */
    OS_HOOK_MEM_DAMAGE, /* 踩内存处理钩子 */

    OS_HOOK_TYPE_TOTAL
};

#define OS_SHOOK_TYPE_START ((U32)OS_HOOK_TSK_MON)

enum HookChgType {
    HOOK_ADD_FIRST,
    HOOK_DEL_LAST
};

union TagMhookCb {
    // 在初始化阶段复用为钩子注册数目
    uintptr_t num;
    // 单钩子
    OsVoidFunc sigHook;
    OsVoidFunc *mulHook;
};

typedef U32(*OsHookChgFunc)(U32 hookType, enum HookChgType chgType);

extern union TagMhookCb g_hookCb[OS_HOOK_TYPE_TOTAL];

typedef void *(*MemAllocHook)(enum MoudleId mid, U8 ptNo, U32 size);
extern MemAllocHook g_osMemAlloc;

/*
 * 多钩子添加内部接口
 */
extern U32 OsMhookAdd(U32 hookType, OsVoidFunc hook);

/*
 * 多钩子删除内部接口
 */
extern U32 OsMhookDel(U32 hookType, OsVoidFunc hook);

/*
 * 钩子添加内部接口
 */
extern U32 OsHookAdd(enum HookType hookType, OsVoidFunc hook);

/*
 * 钩子删除内部接口
 */
extern U32 OsHookDel(enum HookType hookType, OsVoidFunc hook);

/*
 * 多钩子注册内部接口， 为了与原有规格兼容，传入NULL表示删除
 */
extern U32 OsShookReg(U32 hookType, OsVoidFunc hook);

/*
 * 在注册阶段，不同模块通过osMhookReserve接口预留钩子,不影响用户注册钩子数
 * 仅在register阶段使用
 */
extern void OsMhookReserve(U32 hookType, U32 incCnt);

#endif /* PRT_HOOK_EXTERNAL_H */
