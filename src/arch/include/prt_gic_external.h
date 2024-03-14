/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2020-06-16
 * Description: 硬件GIC内部公共头文件
 */
#ifndef PRT_GIC_EXTERNAL_H
#define PRT_GIC_EXTERNAL_H

#include "prt_lib_external.h"
#include "prt_buildef.h"

// 配置中断优先级，每个寄存器对应4个中断
#define GIC_IPRIORITY_INT_NUM     4
// 配置中断优先级，每8bit的高4bit有效，低bit预留
#define GIC_IPRIORITY_HIGH_BIT    4

#if (OS_GIC_VER == 2)
#define GIC_DIST_BASE   0xff841000
#define GIC_CPU_BASE    0xff842000
#define IAR_MASK        0x3FFU
#define GICC_IAR		(GIC_CPU_BASE + 0xc)
#define GICC_EOIR		(GIC_CPU_BASE + 0x10)
#define	GICD_SGIR		(GIC_DIST_BASE + 0xf00)
#elif (OS_GIC_VER == 3)
#define ICC_IAR1_EL1              S3_0_C12_C12_0
#define ICC_EOIR1_EL1             S3_0_C12_C12_1
#define ICC_SGI1R_EL1             S3_0_C12_C11_5
#define ICC_DIR_EL1               S3_0_C12_C11_1
#define ICC_CTLR_EL1              S3_0_C12_C12_4
#endif
#define GIC_REG_BASE              g_gicdBase
#define MAX_SGI_ID                15   // 系统支持的最大SGI的中断号
#define MIN_PPI_ID                16   // 系统支持的最小PPI的中断号
#define MAX_PPI_ID                31   // 系统支持的最大PPI的中断号
#define MAX_SPI_ID                1019 // 系统可支持的最大SPI的中断号
#define MIN_LPI_ID                8192 // 系统可支持的最小LPI的中断号
#define MAX_INT_PRIORITY          0xF  // 安全可配置的最大优先级
#define GIC_INT_ID_MASK           0x3FFU
#define ICC_CTLR_EL1_EOI_MODE     (U32)(1 << 1) // EOImode的值
// 使能/去使能中断，每个寄存器对应32个中断
#define GIC_IENABLE_INT_NUM       32
// 将变参列表字符串化
#define PARAS_TO_STRING(x...)     #x
// 获得变参的值
#define REG_ALIAS(x...)           PARAS_TO_STRING(x)
// 读写GIC寄存器,32位
#define GIC_REG_READ(addr)        (*(volatile U32 *)((uintptr_t)(addr)))
#define GIC_REG_WRITE(addr, data) (*(volatile U32 *)((uintptr_t)(addr)) = (U32)(data))

enum GicIntState {
    GIC_DISABLE = 0,
    GIC_ENABLE = 1
};

/* 存放Core Map值 */
union GicCoreMap {
    struct {
        U64 aff0  : 8;  // bit[7:0]
        U64 aff1  : 8;  // bit[15:8]
        U64 aff2  : 8;  // bit[23:16]
        U64 mt    : 1;  // bit[24] 0:single-thread 核号取AFF0, 1: muti-thread 模式，核号取AFF1
        U64 rsvd0 : 5;  // bit[29:25]
        U64 uni   : 1;  // bit[30]
        U64 rsvd1 : 1;  // bit[31]
        U64 aff3  : 8;  // bit[39:32]
        U64 rsvd2 : 24; // bit[63~40]
    } bits;
    U64    value;
};

/*
 * 描述: 按bit设置GIC寄存器, Read-Modify-Write方式
 */
OS_SEC_ALW_INLINE INLINE void OsGicRmwReg(uintptr_t base, U32 intNum, U32 intId, U32 val)
{
    uintptr_t regAddr;
    U32 regVal;
    U32 offset;
    U32 bitWidth;
    U32 bitMask;

    // 每个寄存器(32位)可配置idNum个硬中断
    bitWidth = OS_WORD_BIT_NUM / intNum;        // 每个硬中断对应的bit位宽:32 / idNum
    bitMask  = (1u << bitWidth) - 1u;       // 每个硬中断对应的bit掩码
    regAddr = base + ((intId / intNum) * sizeof(U32));
    offset  = ((intId % intNum) * bitWidth);

    regVal = GIC_REG_READ(regAddr);
    regVal &= ~(bitMask << offset);         // 清除旧值
    regVal |= ((val & bitMask) << offset);  // 设置新值
    GIC_REG_WRITE(regAddr, regVal);
}

/*
 * 描述: 按bit设置GIC寄存器, 直接写，写前不需要先读
 */
OS_SEC_ALW_INLINE INLINE void OsGicSetReg(uintptr_t base, U32 intNum, U32 intId, U32 val)
{
    uintptr_t regAddr;
    U32 regVal;
    U32 offset;
    U32 bitWidth;
    U32 bitMask;

    // 每个寄存器(32位)可配置idNum个硬中断
    bitWidth = OS_WORD_BIT_NUM / intNum;        // 每个硬中断对应的bit位宽:32 / idNum
    bitMask  = (1u << bitWidth) - 1u;       // 每个硬中断对应的bit掩码
    regAddr = base + ((intId / intNum) * sizeof(U32));
    offset  = ((intId % intNum) * bitWidth);

    regVal = ((val & bitMask) << offset);  // 设置新值
    GIC_REG_WRITE(regAddr, regVal);
}

/*
 * 描述: 按bit读取GIC寄存器的值
 */
OS_SEC_ALW_INLINE INLINE U32 OsGicGetReg(uintptr_t base, U32 idNum, U32 id)
{
    U64 regAddr;
    U32 regVal;
    U32 offset;
    U32 bitWidth;
    U32 bitMask;

    // 每个寄存器有idNum个id，每个id对应的bit位数为sizeof(U32) / idNum
    bitWidth = OS_WORD_BIT_NUM / idNum;
    bitMask  = (1u << bitWidth) - 1u;
    regAddr = base + ((id / idNum) * sizeof(U32));
    offset  = ((id % idNum) * bitWidth);

    regVal = GIC_REG_READ(regAddr);

    return (regVal & (bitMask << offset)) >> offset;
}

/* GIC基地址 */
extern uintptr_t g_gicdBase;
/* GICR相对于GIC基地址偏移向量 */
extern uintptr_t g_gicrOffset;
/* GICR核间偏移向量配置 */
extern uintptr_t g_gicrStride;
/* 存放Core Map值 */
#if defined(OS_OPTION_SMP)
extern union GicCoreMap g_gicCoreMap[OS_MAX_CORE_NUM];
#else
extern union GicCoreMap g_gicCoreMap;
#endif

extern void OsGicEnableInt(U32 intId);
extern void OsGicDisableInt(U32 intId);
extern void OsGicTrigIntToCores(U32 intId, U32 targetList);
extern void OsGicSetTargetId(U32 intId, U32 targetId);
extern U32 OsGicGetPriority(U32 intId);
extern U32 OsGicSetPriority(U32 intId, U32 priority);

extern enum GicIntState OsGicdGetIntState(U32 intId);
extern void OsGicdEnableInt(U32 intId);
extern void OsGicdDisableInt(U32 intId);
extern void OsGicdSetPriority(U32 intId, U32 priority);
extern void OsGicdCfgTargetId(U32 intId, U32 targetId);
extern U32 OsGicdGetPriority(U32 intId);

extern enum GicIntState OsGicrGetIntState(U32 coreId, U32 intId);
extern void OsGicrEnableInt(U32 coreId, U32 intId);
extern void OsGicrDisableInt(U32 coreId, U32 intId);
extern void OsGicrClearPendingBit(U32 coreId, U32 intId);
extern void OsGicrSetPriority(U32 coreId, U32 intId, U32 priority);
extern U32 OsGicrGetPriority(U32 coreId, U32 intId);
extern bool OsGicIsSpi(U32 intId);

extern void OsGicrLpiEnableInt(U32 intId);
extern void OsGicrLpiDisableInt(U32 intId);
extern void OsGicrLpiSetPriority(U32 intId, U32 priority);
extern U32 OsGicrLpiGetPriority(U32 intId);

extern U32 g_cfgPrimaryCore;
#endif /* PRT_GIC_EXTERNAL_H */
