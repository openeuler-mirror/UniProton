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
 * Create: 2024-01-27
 * Description: Task模块
 */
#ifndef PRT_SMP_PSCI_INTERNAL_H
#define PRT_SMP_PSCI_INTERNAL_H

#include "prt_hwi_external.h"
extern U32 OsArmSmccSmc(U64 a0, U64 a1, U64 a2, U64 a3,
                        U64 a4, U64 a5, U64 a6, U64 a7);
extern void OsElxState(void);
#if defined(OS_OPTION_POWEROFF)
extern void OsCpuPowerOff(void);
#endif
#define BITS_PER_CHAR 8

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PSCI_CPU_STATE_ON       0U
#define PSCI_CPU_STATE_OFF       1U

/* PSCI V0.2 interface */
#define PSCI_FN_BASE                0x84000000
#define PSCI_FN(val)                (PSCI_FN_BASE + (val))
#define PSCI_64BIT                  0X40000000
#define PSCI_FN64_BASE              (PSCI_FN_BASE + PSCI_64BIT)
#define PSCI_FN64(n)                (PSCI_FN64_BASE + (n))

#define PSCI_FN_CPU_OFF             PSCI_FN(2)
#define PSCI_FN_CPU_ON              PSCI_FN(3)
#define PSCI_FN_AFFINITY_INFO       PSCI_FN(4)

#define PSCI_FN64_CPU_ON            PSCI_FN64(3)
#define PSCI_FN64_AFFINITY_INFO     PSCI_FN64(4)

#if defined(OS_OPTION_CPU64)
#define PSCI_CPU_OFF                PSCI_FN_CPU_OFF
#define PSCI_CPU_ON                 PSCI_FN64_CPU_ON
#define PSCI_AFFINITY_INFO          PSCI_FN64_AFFINITY_INFO
#else
#define PSCI_CPU_OFF                PSCI_FN_CPU_OFF
#define PSCI_CPU_ON                 PSCI_FN_CPU_ON
#define PSCI_AFFINITY_INFO          PSCI_FN_AFFINITY_INFO
#endif

#if defined(OS_ARCH_ARMV8_AARCH32)
#define PSCI_CPU_ON_ENTRY           (uintptr_t)OsResetVector
#else
#define PSCI_CPU_ON_ENTRY           (uintptr_t)OsElxState
#endif

/* MPIDR相关操作 */
#define OS_MPIDR_BASE  OsGetMpidr()
#define MPIDR_VALID_COREID_MASK 0xffffffU
/* mpidr寄存器bit24 MT 为1，则cpuId间隔1 << 8, 否则间隔 1 << 0 */
#define OS_MPIDR_OFFSET ((U32)(bool)(OS_MPIDR_BASE & OS_BIT24_MASK) * BITS_PER_CHAR)
/* 物理核号 */
#define OS_MPIDR_VALID_COREID (OS_MPIDR_BASE & MPIDR_VALID_COREID_MASK)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
#endif