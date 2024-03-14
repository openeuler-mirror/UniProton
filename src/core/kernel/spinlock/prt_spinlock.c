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
 * Create: 2024-01-23
 * Description: spinlock模块的C文件。
 */
#include "prt_raw_spinlock_external.h"
#include "prt_task_external.h"
#include "prt_sys_external.h"

#if !defined(OS_OPTION_SMP)
#define OS_TSK_SPINLOCK()
#define OS_TSK_SPINUNLOCK() 
#else
#define OS_TSK_SPINLOCK() OS_TSK_LOCK()
#define OS_TSK_SPINUNLOCK() OS_TSK_UNLOCK()

/* 暂且通过prt_spinlock把g_mcInitGuard公布出去，*/
#if(OS_MAX_CORE_NUM > 1)
OS_SEC_L4_INSTSH_DATA volatile uintptr_t g_mcInitGuard = 0;
#endif

OS_SEC_L4_TEXT U32 PRT_SplLockInit(struct PrtSpinLock *spinLock)
{
    if(spinLock == NULL) {
        return OS_ERRNO_SPL_ALLOC_ADDR_INVALID;
    }
    return OsSplLockInit(spinLock);
}
#endif

#if(OS_MAX_CORE_NUM > 1)
/* 应用使用的spinlock，禁止调度 */
OS_SEC_L0_TEXT void PRT_SplLock(struct PrtSpinLock *spinLock)
{
    volatile uintptr_t *addr;

    addr = &spinLock->rawLock;

    OS_TSK_SPINLOCK();

    OsSplLock(addr);
}

/* 应用使用的spinlock，禁止调度 */
OS_SEC_L0_TEXT void PRT_SplUnlock(struct PrtSpinLock *spinLock)
{
    volatile uintptr_t *addr;

    addr = &spinLock->rawLock;

    OsSplUnlock(addr);

    OS_TSK_SPINUNLOCK();
}

/* 应用使用的spinlock，禁止调度，类似spin_lock_irqsave */
OS_SEC_L0_TEXT uintptr_t PRT_SplIrqLock(struct PrtSpinLock *spinLock)
{
    uintptr_t intSave;
    volatile uintptr_t *addr;

    addr = &spinLock->rawLock;

    intSave = OsIntLock();

    OS_TSK_SPINLOCK();

    OsSplLock(addr);

    return intSave;
}

/* 应用使用的spinlock，禁止调度，类似spin_unlock_irqrestore */
OS_SEC_L0_TEXT void PRT_SplIrqUnlock(struct PrtSpinLock *spinLock, uintptr_t intSave)
{
    volatile uintptr_t *addr;

    addr = &spinLock->rawLock;

    OsSplUnlock(addr);
    OS_TSK_SPINUNLOCK();
    OsIntRestore(intSave);
}

#if defined(OS_OPTION_SMP)
/*
 * 描述，读写互斥，读者原子++，遇写wfe
 */
OS_SEC_L2_TEXT uintptr_t PRT_SplIrqReadLock(struct PrtSpinLock *spinLock)
{
    uintptr_t intSave;
    volatile uintptr_t *addr = &spinLock->rawLock;
    intSave = PRT_IntLock();
    OS_TSK_LOCK();
    OsSplReadLock(addr);
    return intSave;
}

OS_SEC_L2_TEXT void PRT_SplIrqReadUnlock(struct PrtSpinLock *spinLock, uintptr_t intSave)
{
    volatile uintptr_t *addr = &spinLock->rawLock;
    OsSplReadUnlock(addr);
    OsTskUnlock();
    PRT_IntRestore(intSave);
    return;
}

OS_SEC_L2_TEXT uintptr_t PRT_SplIrqWriteLock(struct PrtSpinLock *spinLock)
{
    uintptr_t intSave;
    volatile uintptr_t *addr = &spinLock->rawLock;
    intSave = PRT_IntLock();
    OS_TSK_LOCK();
    OsSplWriteLock(addr);
    return intSave;
}

OS_SEC_L2_TEXT void PRT_SplIrqWriteUnlock(struct PrtSpinLock *spinLock, uintptr_t intSave)
{
    volatile uintptr_t *addr = &spinLock->rawLock;
    OsSplWriteUnlock(addr);
    OsTskUnlock();
    PRT_IntRestore(intSave);
    return;
}

#endif
#else
/* 应用使用的spinlock，禁止调度，类似spin_unlock_irqrestore */
OS_SEC_L0_TEXT void PRT_SplLock(struct PrtSpinLock *spinLock)
{
    (void)spinLock;
}

/* 应用使用的spinlock，禁止调度，类似spin_unlock_irqrestore */
OS_SEC_L0_TEXT void PRT_SplUnlock(struct PrtSpinLock *spinLock)
{
    (void)spinLock;
}

/* 应用使用的spinlock，禁止调度，类似spin_unlock_irqrestore */
OS_SEC_L0_TEXT uintptr_t PRT_SplIrqLock(struct PrtSpinLock *spinLock)
{
    (void)spinLock;
    return PRT_IntLock();
}

/* 应用使用的spinlock，禁止调度，类似spin_unlock_irqrestore */
OS_SEC_L0_TEXT void PRT_SplIrqUnlock(struct PrtSpinLock *spinLock, uintptr_t intSave)
{
    (void)spinLock;
    PRT_IntRestore(intSave);
}
#endif