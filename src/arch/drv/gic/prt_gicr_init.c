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
 * Description: 硬件GICR相关的处理。
 */
#include "prt_gic_external.h"
#include "prt_gic_internal.h"
#include "prt_attr_external.h"

/* GICR_CTRL */
union GicrCtrl {
    struct {
        U32 enLpis : 1;  // bit[0] LPIs中断使能控制寄存器
        U32 res2   : 2;  // bit[2:1]
        U32 rwp    : 1;  // bit[3] 标识前面一次写GICR寄存器的配置信息是否生效
        U32 res1   : 20; // bit[23:4]
        U32 dpg0   : 1;  // bit[24]
        U32 dpg1ns : 1;  // bit[25]
        U32 dpg1s  : 1;  // bit[26]
        U32 res0   : 4;  // bit[30:27]
        U32 uwp    : 1;  // bit[31] GICD对Upstream Write和Generate SGI是否可见
    } bits;
    U32    value;
};

/*
 * 描述: 等待写寄存器操作生效
 */
OS_SEC_TEXT void OsGicrWaitCfgWork(U32 coreId)
{
    union GicrCtrl gicrCtrl;
    uintptr_t regAddr;

    regAddr = GICR_CTRL_ADDR + (coreId * GICR_ADDR_OFFSET_PER_CORE);
    do {
        gicrCtrl.value = GIC_REG_READ(regAddr);
    } while (gicrCtrl.bits.rwp == 1);
}

/*
 * 描述: 获取PPI, SGI, NNSPI的使能状态, 调用者保证入参的有效性
 */
OS_SEC_TEXT enum GicIntState OsGicrGetIntState(U32 coreId, U32 intId)
{
    // 每个寄存器对应32个中断
    return OsGicGetReg(GICR_ISENABLER0_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                       GIC_IENABLE_INT_NUM, intId);
}

/*
 * 描述: 使能PPI, SGI, NNSPI, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrEnableInt(U32 coreId, U32 intId)
{
    // 每个寄存器对应32个中断，写1使能中断，写0无效
    OsGicSetReg(GICR_ISENABLER0_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                GIC_IENABLE_INT_NUM, intId, 1);

    OsGicrWaitCfgWork(coreId); // 确保操作生效
}

/*
 * 描述: 去使能PPI, SGI, NNSPI, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrDisableInt(U32 coreId, U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICR_ICENABLER0_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                GIC_IENABLE_INT_NUM, intId, 1);

    OsGicrWaitCfgWork(coreId);  // 确保操作生效
}

/*
 * 描述: 清除指定中断号的pending位, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrClearPendingBit(U32 coreId, U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICR_ICPENDR0_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                GIC_IENABLE_INT_NUM, intId, 1);

    OsGicrWaitCfgWork(coreId);  // 确保操作生效
}

/*
 * 描述: 清除指定中断号的pending位, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrClearActiveBit(U32 coreId, U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICR_ICACTIVER0_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                GIC_IENABLE_INT_NUM, intId, 1);

    OsGicrWaitCfgWork(coreId);  // 确保操作生效
}

/*
 * 描述: 设置SGI, PPI, NNSPI中断的优先级, 调用者保证入参的有效性
 */
OS_SEC_L4_TEXT void OsGicrSetPriority(U32 coreId, U32 intId, U32 priority)
{
    // 每个寄存器对应4个中断, 每8bit中的高4bit对应一个中断优先级
    OsGicRmwReg(GICR_IPRIORITY_SGI_S_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                GIC_IPRIORITY_INT_NUM, intId, priority << GIC_IPRIORITY_HIGH_BIT);
}

/*
 * 描述: 获取SGI, PPI, NNSPI中断的优先级, 调用者保证入参的有效性
 */
OS_SEC_L2_TEXT U32 OsGicrGetPriority(U32 coreId, U32 intId)
{
    U32 gicrPrio;

    // 每个寄存器（32位）对应4个中断, 即每8bit对应1个中断
    gicrPrio = OsGicGetReg(GICR_IPRIORITY_SGI_S_ADDR + coreId * GICR_ADDR_OFFSET_PER_CORE,
                           GIC_IPRIORITY_INT_NUM, intId);

    return (gicrPrio >> GIC_IPRIORITY_HIGH_BIT); // 4: 每8bit中的高4bit对应一个中断的优先级, 低4bit预留。
}

/*
 * 描述: 使能LPI, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrLpiEnableInt(U32 intId)
{
    /* todo: */
    return;
}

/*
 * 描述: 去使能LPI, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicrLpiDisableInt(U32 intId)
{
    /* todo: */
    return;
}

OS_SEC_L4_TEXT void OsGicrLpiSetPriority(U32 intId, U32 priority)
{
    /* todo: */
    return;
}
OS_SEC_L2_TEXT U32 OsGicrLpiGetPriority(U32 intId)
{
    /* todo: */
    return 0x20;
}
