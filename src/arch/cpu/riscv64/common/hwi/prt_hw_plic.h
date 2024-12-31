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
 * Create: 2024-01-14
 * Description: RISCV64 中断控制器PLIC对内驱动头文件
 */
#ifndef PRT_HW_PLIC_H
#define PRT_HW_PLIC_H

#include "prt_buildef_common.h"
#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_attr_external.h"
#include "../common/prt_riscv.h"

#if (OS_CPU_TYPE == OS_RV64_VIRT)
#include "./board/qemu_rv64virt/platform.h"
#endif

#if (OS_CPU_TYPE == OS_RV64_D1S)
#include "./board/ds_d1s/platform.h"
#endif

#if (OS_CPU_TYPE == OS_RV64_MILKVDUOL)
#include "./board/milkvduol/platform.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#define PLIC_CLAIM_NO_PENDING_VAL 0U
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

#define PLIC_GET_PRIO(hwiNum)   ((*(U32 *)(PLIC + (hwiNum)*4)))

#define PLIC_SET_PRIO(hwiNum, hwiPrio) (*(U32 *)(PLIC + (hwiNum)*4) = (hwiPrio))

#define PLIC_GET_PENDDING(hwiNum) ((*((U32 *)PLIC_PENDING + ((hwiNum)>>5))) & (1<<((hwiNum) & 0b11111)))

#define PLIC_SET_PENDDING(hwiNum) 

extern U32 PLIC_CLAIM(U8 mode, U8 hart);

extern void PLIC_COMPLETE(U8 mode, U8 hart, U32 irq);

extern void PLIC_ENI(U8 mode, U8 hart, U32 irq);

extern void PLIC_DSI(U8 mode, U8 hart, U32 irq);




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#endif 
