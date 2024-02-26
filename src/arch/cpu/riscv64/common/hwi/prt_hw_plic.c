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
 * Description: RISCV64 中断控制器PLIC对内驱动文件
 */
#include "prt_hw_plic.h"

OS_SEC_L4_TEXT U32 PLIC_CLAIM(U8 mode, U8 hart)
{
    if(mode == CPU_M_MODE) {
        return *(U32 *)PLIC_MCLAIM(hart);
    }
    else {
        return *(U32 *)PLIC_SCLAIM(hart);
    }
}

OS_SEC_L4_TEXT void PLIC_COMPLETE(U8 mode, U8 hart, U32 irq)
{
    if(mode == CPU_M_MODE) {
        *(U32 *)PLIC_MCLAIM(hart) = irq;
    }
    else {
        *(U32 *)PLIC_SCLAIM(hart) = irq;
    }
}

OS_SEC_L4_TEXT void PLIC_ENI(U8 mode, U8 hart, U32 irq)
{
    if(mode == CPU_M_MODE) {
        *((U32 *)PLIC_MENABLE(hart) + (irq>>5)) |= (1<<(irq & 0b11111));
    }
    else {
        *((U32 *)PLIC_SENABLE(hart) + (irq>>5)) |= (1<<(irq & 0b11111));
    }
}

OS_SEC_L4_TEXT void PLIC_DSI(U8 mode, U8 hart, U32 irq)
{
    if(mode == CPU_M_MODE) {
        *((U32 *)PLIC_MENABLE(hart) + (irq>>5)) &= (~(1<<(irq & 0b11111)));
    }
    else {
        *((U32 *)PLIC_SENABLE(hart) + (irq>>5)) &= (~(1<<(irq & 0b11111)));
    }
}