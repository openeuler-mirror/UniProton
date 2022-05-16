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
 * Description: hook模块的初始化文件
 */
#include "prt_idle.h"
#include "prt_cpu_external.h"
#include "prt_hook_internal.h"

OS_SEC_BSS union TagMhookCb g_hookCb[OS_HOOK_TYPE_TOTAL];

/*
 * 有些模块在钩子变化时，希望得到及时通知。比如tick钩子需要做g_tickDispatcher置换等
 */
OS_SEC_BSS OsHookChgFunc g_hookChgHandler[OS_HOOK_TYPE_TOTAL];
// 注册钩子用的锁
OS_SEC_BSS volatile uintptr_t g_hookRegLock;
OS_SEC_BSS MemAllocHook g_osMemAlloc;

/*
 * 描述：内核钩子模块注册
 */
OS_SEC_L4_TEXT U32 OsHookRegister(struct HookModInfo *modInfo)
{
    U32 hookIndex;

    for (hookIndex = 0; hookIndex < (U32)OS_HOOK_TYPE_NUM; hookIndex++) {
        g_hookCb[hookIndex].num += modInfo->maxNum[hookIndex];
    }

    return OS_OK;
}

/*
 * 描述：多钩子使用预留
 */
OS_SEC_L4_TEXT void OsMhookReserve(U32 hookType, U32 incCnt)
{
    g_hookCb[hookType].num += incCnt;
}

/*
 * 描述：内核钩子模块初始化
 */
OS_SEC_L4_TEXT U32 OsHookConfigInit(void)
{
    U32 size = 0;
    U32 hookCnt;
    U32 hookIndex;
    OsVoidFunc *hooks = NULL;

    for (hookIndex = 0; hookIndex < OS_SHOOK_TYPE_START; hookIndex++) {
        hookCnt = g_hookCb[hookIndex].num;
        if (hookCnt > 0) {
            // 增加一个用于保护节点
            size += hookCnt + 1;
        }
    }

    if (size == 0) {  // 没有配置任何钩子
        return OS_OK;
    }

    size = (U32)(size * sizeof(OsVoidFunc));

    if (g_osMemAlloc != NULL) {
        hooks = g_osMemAlloc(OS_MID_HOOK, OS_MEM_DEFAULT_FSC_PT, size);
    }

    if (hooks == NULL) {
        return OS_ERRNO_HOOK_NO_MEMORY;
    }

    if (memset_s((void *)hooks, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    for (hookIndex = 0; hookIndex < OS_SHOOK_TYPE_START; hookIndex++) {
        hookCnt = g_hookCb[hookIndex].num;
        if (hookCnt == 0) {
            continue;
        }

        g_hookCb[hookIndex].mulHook = hooks;
        *(hooks + hookCnt) = (OsVoidFunc)OS_MHOOK_BOUNDARY;
        hooks += hookCnt + 1;
    }

    OS_SPIN_LOCK_INIT(g_hookRegLock);

    return OS_OK;
}

/*
 * 描述：多钩子添加
 */
OS_SEC_L4_TEXT U32 OsMhookAdd(U32 hookType, OsVoidFunc hook)
{
    uintptr_t intSave;
    OsVoidFunc *mHook = NULL;
    U32 ret = OS_OK;
    U32 hookCnt = 0;
    OsVoidFunc *add = NULL;

    mHook = g_hookCb[hookType].mulHook;

    /* 配置多钩子数为0，返回错误码 */
    if (mHook == NULL) {
        return OS_ERRNO_HOOK_NOT_CFG;
    }

    HOOK_ADD_IRQ_LOCK(intSave);

    while (OS_MHOOK_IS_VALID(*mHook)) {
        /* 如果是之前删除过的Dead节点，直接跳过 */
        if (*mHook == OS_MHOOK_NODE_DEAD) {
            /* 记录第一个Dead节点，作为后面新添加节点 */
            if (add == NULL) {
                add = mHook;
            }
            mHook++;
            continue;
        }

        /* 如果该钩子已经注册，返回错误码 */
        if (*mHook == hook) {
            HOOK_ADD_IRQ_UNLOCK(intSave);
            return OS_ERRNO_HOOK_EXISTED;
        }
        mHook++;
        hookCnt++;
    }

    /* 如果新添加节点没有从Dead节点上分配，从尾部划取一个节点 */
    if (add == NULL) {
        if (!OS_MHOOK_NOT_BOUNDARY(*mHook)) {
            HOOK_ADD_IRQ_UNLOCK(intSave);
            return OS_ERRNO_HOOK_FULL;
        }
        add = mHook;
    }

    /* 调用钩子变更通知钩子 */
    if ((hookCnt == 0) && (g_hookChgHandler[hookType] != NULL)) {
        ret = g_hookChgHandler[hookType](hookType, HOOK_ADD_FIRST);
    }

    /* 添加钩子 */
    if (ret == OS_OK) {
        *add = hook;
    }

    HOOK_ADD_IRQ_UNLOCK(intSave);
    return ret;
}

/*
 * 描述：多钩子删除
 */
OS_SEC_L4_TEXT U32 OsMhookDel(U32 hookType, OsVoidFunc hook)
{
    uintptr_t intSave;
    OsVoidFunc *mHook = NULL;
    U32 ret;
    U32 hookCnt = 0;
    OsVoidFunc *del = NULL;

    mHook = g_hookCb[hookType].mulHook;
    if (mHook == NULL) {
        return OS_ERRNO_HOOK_NOT_CFG;
    }

    HOOK_DEL_IRQ_LOCK(intSave);

    while (OS_MHOOK_IS_VALID(*mHook)) {
        if (*mHook == OS_MHOOK_NODE_DEAD) {
            mHook++;
            continue;
        }

        if (*mHook == hook) {
            // 可断言del为空
            del = mHook;
        }

        mHook++;
        hookCnt++;
    }

    if (del == NULL) {
        HOOK_DEL_IRQ_UNLOCK(intSave);
        return OS_ERRNO_HOOK_NOT_EXISTED;
    }

    if ((hookCnt == 1) && (g_hookChgHandler[hookType] != NULL)) {
        ret = g_hookChgHandler[hookType](hookType, HOOK_DEL_LAST);
        if (ret != OS_OK) {
            HOOK_DEL_IRQ_UNLOCK(intSave);
            return ret;
        }
    }

    // 如果是最后一个，置为FREE，避免每次钩子调用遍历。 否则置为DEAD态。
    if (OS_MHOOK_IS_VALID(*(del + 1))) {
        *del = OS_MHOOK_NODE_DEAD;
    } else {
        *del-- = OS_HOOK_EMPTY;
        while ((del >= g_hookCb[hookType].mulHook) && (*del == OS_MHOOK_NODE_DEAD)) {
            *del-- = OS_HOOK_EMPTY;
        }
    }

    HOOK_DEL_IRQ_UNLOCK(intSave);
    return OS_OK;
}

/*
 * 描述：钩子添加
 */
OS_SEC_L4_TEXT U32 OsHookAdd(enum HookType hookType, OsVoidFunc hook)
{
    if ((U32)hookType >= (U32)OS_HOOK_TYPE_NUM) {
        return OS_ERRNO_HOOK_TYPE_INVALID;
    }

    if (hook == NULL) {
        return OS_ERRNO_HOOK_PTR_NULL;
    }

    return OsMhookAdd((U32)hookType, hook);
}

/*
 * 描述：钩子删除
 */
OS_SEC_L4_TEXT U32 OsHookDel(enum HookType hookType, OsVoidFunc hook)
{
    if ((U32)hookType >= (U32)(OS_HOOK_TYPE_NUM)) {
        return OS_ERRNO_HOOK_TYPE_INVALID;
    }

    if (hook == NULL) {
        return OS_ERRNO_HOOK_PTR_NULL;
    }

    return OsMhookDel((U32)hookType, hook);
}

/*
 * 描述：单钩子注册
 */
OS_SEC_L4_TEXT U32 OsShookReg(U32 hookType, OsVoidFunc hook)
{
    uintptr_t intSave;
    U32 ret = OS_OK;

    if (!OS_IS_SHOOK_TYPE(hookType)) {
        return OS_ERRNO_HOOK_TYPE_INVALID;
    }

    HOOK_ADD_IRQ_LOCK(intSave);

    if (hook == NULL) {  // unreg
        if (g_hookCb[hookType].sigHook == NULL) {
            HOOK_ADD_IRQ_UNLOCK(intSave);
            return OS_OK;
        }

        if (g_hookChgHandler[hookType] != NULL) {
            ret = g_hookChgHandler[hookType](hookType, HOOK_DEL_LAST);
        }

        if (ret == OS_OK) {
            g_hookCb[hookType].sigHook = NULL;
        }
    } else {
        if (g_hookCb[hookType].sigHook != NULL) {
            HOOK_ADD_IRQ_UNLOCK(intSave);
            return OS_ERRNO_HOOK_FULL;
        }

        if (g_hookChgHandler[hookType] != NULL) {
            ret = g_hookChgHandler[hookType](hookType, HOOK_ADD_FIRST);
        }

        if (ret == OS_OK) {
            g_hookCb[hookType].sigHook = hook;
        }
    }

    HOOK_ADD_IRQ_UNLOCK(intSave);
    return ret;
}

/*
 * 描述：前置钩子添加
 */
OS_SEC_L4_TEXT U32 PRT_IdleAddPrefixHook(IdleHook hook)
{
    if (hook == NULL) {
        return OS_ERRNO_SYS_PTR_NULL;
    }

    return OsShookReg(OS_HOOK_IDLE_PREFIX, (OsVoidFunc)hook);
}
