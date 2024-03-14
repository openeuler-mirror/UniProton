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
 * Create: 2024-04-08
 * Description: spinlock内部头文件。
 */
#ifndef PRT_RAW_SPINLOCK_EXTERNAL_H
#define PRT_RAW_SPINLOCK_EXTERNAL_H

#include "prt_atomic.h"

#include "prt_cpu_external.h"

#define OS_MCMUTEX_LOCK(hwSemId, addr)      \
    do {                                    \
        (void)(hwSemId);                       \
        OsSplLock(addr);                 \
    } while (0)
#define OS_MCMUTEX_UNLOCK(hwSemId, addr)    \
    do {                                    \
        (void)(hwSemId);                       \
        OsSplUnlock(addr);               \
    } while (0)

#define OS_MCMUTEX_IRQ_LOCK(hwSemId, addr, intSave)    \
    do {                                               \
        (void)hwSemId;                                 \
        (intSave) = PRT_IntLock();                     \
        OsSplLock(addr);                             \
    } while (0)

#define OS_MCMUTEX_IRQ_UNLOCK(hwSemId, addr, intSave)    \
    do {                                              \
        (void)hwSemId;                                \
        OsSplUnlock(addr);                         \
        PRT_IntRestore(intSave);                      \
    } while (0)


extern volatile uintptr_t g_mcInitGuard;

#define OS_SPIN_LOCK(lockVar)   OsSplLock((volatile uintptr_t *)(lockVar))
#define OS_SPIN_UNLOCK(lockVar)   OsSplUnlock((volatile uintptr_t *)(lockVar))
#define OS_SPIN_TRY_LOCK(lockVar)   OsSplTryLock((uintptr_t *)(lockVar))
#endif /* PRT_RAW_SPINLOCK_EXTERNAL_H */