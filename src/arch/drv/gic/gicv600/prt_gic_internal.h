/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022/11/18
 * Description: shared data declaration
 */

#ifndef PRT_GIC_INTERNAL_H
#define PRT_GIC_INTERNAL_H

#include "../prt_gic_common_internal.h"

/*
 * 模块间宏定义
 */
/* 各个board有差异的中断配置 */
#define MAX_NNSPI_ID             31 // 系统可支持的最大NNSPI的中断号，为了代码归一，不支持NNSPI场景配置成MAX_PPI_ID相同
#define MIN_SPI_ID               32 // 系统可支持的最小SPI的中断号
#define GICD_SPI_IROUTERN_L_ADDR (GIC_GICD_BASE + 0x6000U) // GIC亲和性配置寄存器

/*
 * 描述: GicV3 根据中断号获取全局中断亲和配置寄存器地址
 */
OS_SEC_ALW_INLINE INLINE uintptr_t OsGicGetRouterAddr(U32 intId)
{
    return GICD_SPI_IROUTERN_L_ADDR + (uintptr_t)(sizeof(uintptr_t) * intId);
}

#endif /* PRT_GIC_INTERNAL_H */
