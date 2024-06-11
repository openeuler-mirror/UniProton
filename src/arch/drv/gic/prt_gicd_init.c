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
 * Description: 硬件GICD相关的处理。
 */
#include "prt_gic_external.h"
#include "prt_gic_internal.h"
#include "prt_attr_external.h"

/* GICD_ROUTER */
union GicdRouter {
    struct {
        U64 af0  : 8;  // bit[7:0]
        U64 af1  : 8;  // bit[15:8]
        U64 af2  : 8;  // bit[23:16]
        U64 res0 : 7;  // bit[30:24]
        U64 mode : 1;  // bit[31]
        U64 af3  : 8;  // bit[39:32]
        U64 res1 : 24; // bit[40~63]
    } bits;
    U64    value;
};

/* GICD_CTRL */
union GicdCtrl {
    struct {
        U32 enG0S  : 1;  // bit[0] Enable Secure Group0 interrupt, Group0安全中断使能
        U32 enG1Ns : 1;  // bit[1] Enable Non-Secure Group1 interrupt, Group1非安全中断使能
        U32 enG1S  : 1;  // bit[2] Enable Secure Group1 interrupt, Group1安全中断使能
        U32 res1   : 1;  // bit[3]
        U32 areS   : 1;  // bit[4] Affinity Routing Eanble（for Secure）
        U32 areNs  : 1;  // bit[5] 非安全状态下的关联路由使能
        U32 ds     : 1;  // bit[6] Disable Security,当DS置1时，非安全操作可以访问Group0的配置寄存器
        U32 res0   : 24; // bit[30:7]
        U32 rwp    : 1;  // bit[31] 标识前面一次写GICD_CTLR寄存器的配置信息是否生效,0表示已生效
    } bits;
    U32    value;
};

/*
 * 描述: 等待寄存器配置成功
 */
OS_SEC_TEXT void OsGicdWaitCfgWork(void)
{
    union GicdCtrl gicdCtrl;

    do {
        gicdCtrl.value = GIC_REG_READ(GICD_CTLR_S_ADDR);
    } while (gicdCtrl.bits.rwp == 1);
}

/*
 * 描述: 获取SPI中断的使能状态, 调用者保证入参的有效性
 */
OS_SEC_TEXT enum GicIntState OsGicdGetIntState(U32 intId)
{
    // 每个寄存器对应32个中断
    return OsGicGetReg(GICD_ISENABLER0_ADDR, GIC_IENABLE_INT_NUM, intId);
}

/*
 * 描述: 使能SPI中断, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicdEnableInt(U32 intId)
{
    // 每个寄存器对应32个中断，写1使能中断，写0无效
    OsGicSetReg(GICD_ISENABLER0_ADDR, GIC_IENABLE_INT_NUM, intId, 1);

    OsGicdWaitCfgWork();
}

/*
 * 描述: 去使能SPI中断, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicdDisableInt(U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICD_ICENABLER0_ADDR, GIC_IENABLE_INT_NUM, intId, 1);

    OsGicdWaitCfgWork();
}

/*
 * 描述: 清除SPI中断pending位, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicdClearPendingBit(U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICD_ICPENDR0_ADDR, GIC_IENABLE_INT_NUM, intId, 1);

    OsGicdWaitCfgWork();
}

/*
 * 描述: 清除SPI中断Active位, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicdClearActiveBit(U32 intId)
{
    // 每个寄存器对应32个中断，写1去使能中断，写0无效
    OsGicSetReg(GICD_ICACTIVER0_ADDR, GIC_IENABLE_INT_NUM, intId, 1);

    OsGicdWaitCfgWork();
}

/*
 * 描述: 设置SPI中断的优先级, 调用者保证入参的有效性
 */
OS_SEC_L4_TEXT void OsGicdSetPriority(U32 intId, U32 priority)
{
    // 每个寄存器对应4个中断, 每8bit中的高4bit对应一个中断优先级
    OsGicRmwReg(GICD_SGI_IPRIORITY_S_ADDR, GIC_IPRIORITY_INT_NUM, intId, priority << GIC_IPRIORITY_HIGH_BIT);
}

/*
 * 描述: 获取SPI中断的优先级, 调用者保证入参的有效性
 */
OS_SEC_L4_TEXT U32 OsGicdGetPriority(U32 intId)
{
    U32 priority;

    // 每个寄存器对应4个中断
    priority = OsGicGetReg(GICD_SGI_IPRIORITY_S_ADDR, GIC_IPRIORITY_INT_NUM, intId);

    return priority >> GIC_IPRIORITY_HIGH_BIT; // 每8bit中的高4bit对应一个中断的优先级, 低4bit预留。
}

/*
 * 描述: 配置SPI中断的目标核, 调用者保证入参的有效性
 */
OS_SEC_TEXT void OsGicdCfgTargetId(U32 intId, U32 targetId)
{
    union GicdRouter gicdRouter;
    uintptr_t addr;

    addr = OsGicGetRouterAddr(intId);
    gicdRouter.value = GIC_REG_READ(addr);

    /* 当前仅支持1-1模式 */
    gicdRouter.bits.mode = 0x0;
#if defined(OS_OPTION_SMP)
    if (g_gicCoreMap[PRT_GetPrimaryCore()].bits.mt == 0) {
#else
    if (g_gicCoreMap.bits.mt == 0) {
#endif
        /* single-thread 模式下，核号取AFF0 */
        gicdRouter.bits.af1 = 0;
        gicdRouter.bits.af0 = targetId;
    } else {
        /*  muti-thread 模式下，核号取AFF1 */
        gicdRouter.bits.af0 = 0;
        gicdRouter.bits.af1 = targetId;
    }

    GIC_REG_WRITE(addr, gicdRouter.value);
}
