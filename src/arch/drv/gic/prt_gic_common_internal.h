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
 * Create: 2022-11-18
 * Description: shared data declaration
 */

#ifndef PRT_GIC_COMMON_INTERNAL_H
#define PRT_GIC_COMMON_INTERNAL_H

#include "prt_gic_external.h"

/*
 * 模块间宏定义
 */
/* 各个平台公共的宏 */
#define GIC_GICD_BASE                 (GIC_REG_BASE + 0x0)
#define GICD_CTLR_S_ADDR              (GIC_GICD_BASE + 0x0000)
#define GICD_ISENABLER0_ADDR          (GIC_GICD_BASE + 0x0100)
#define GICD_ICENABLER0_ADDR          (GIC_GICD_BASE + 0x0180)
#define GICD_ICPENDR0_ADDR            (GIC_GICD_BASE + 0x0280)
#define GICD_ICACTIVER0_ADDR          (GIC_GICD_BASE + 0x0380)
#define GICD_SGI_IPRIORITY_S_ADDR     (GIC_GICD_BASE + 0x0400)
#define GICD_SGI_ICFGR_ADDR           (GIC_GICD_BASE + 0x0C00)

#define GIC_GICR_BASE0                (GIC_REG_BASE + g_gicrOffset)
#define GIC_GICR_BASE1                (GIC_GICR_BASE0 + 0x10000)
#define GICR_CTRL_ADDR                (GIC_GICR_BASE0 + 0x0000)
#define GICR_IIDR_ADDR                (GIC_GICR_BASE0 + 0x0004U)
#define GICR_TYPER_ADDR               (GIC_GICR_BASE0 + 0x0008U)  /* 64bit */
#define GICR_STATUSR_ADDR             (GIC_GICR_BASE0 + 0x0010U)
#define GICR_WAKER_ADDR               (GIC_GICR_BASE0 + 0x0014U)
#define GICR_PROPBASER_ADDR           (GIC_GICR_BASE0 + 0x0070U)  /* 64bit */
#define GICR_PENDBASER_ADDR           (GIC_GICR_BASE0 + 0x0078U)  /* 64bit */
#define GICR_ISENABLER0_ADDR          (GIC_GICR_BASE1 + 0x0100)
#define GICR_ICENABLER0_ADDR          (GIC_GICR_BASE1 + 0x0180)
#define GICR_ISPENDR0_ADDR            (GIC_GICR_BASE1 + 0x0200U)
#define GICR_ICPENDR0_ADDR            (GIC_GICR_BASE1 + 0x0280U)
#define GICR_ICACTIVER0_ADDR          (GIC_GICR_BASE1 + 0x0380U)
#define GICR_IPRIORITY_SGI_S_ADDR     (GIC_GICR_BASE1 + 0x0400)
#define GICR_ADDR_OFFSET_PER_CORE     g_gicrStride

#endif /* PRT_GIC_COMMON_INTERNAL_H */
