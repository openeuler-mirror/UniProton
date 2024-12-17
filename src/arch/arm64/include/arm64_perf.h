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
 * Create: 2024-03-11
 * Description: Perf
 */

#ifndef ARM64_PERF_H
#define ARM64_PERF_H

#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef OS_OPTION_PERF
/* Get Caller's pc and fp in non-irq context */
#define OsPerfArchFetchCallerRegs(regs) \
    do { \
        (regs)->pc = (uintptr_t)__builtin_return_address(0); \
        (regs)->fp = (uintptr_t)__builtin_frame_address(0);  \
    } while (0)

/* Get Caller's pc and fp in irq context */
#define OsPerfArchFetchIrqRegs(regs, tcb) \
    do { \
        (regs)->pc = (tcb)->pc; \
        (regs)->fp = (tcb)->fp; \
    } while (0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* ARM64_PERF_H */
