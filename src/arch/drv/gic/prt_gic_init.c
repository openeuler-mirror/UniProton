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
 * Description: 硬件GIC相关的处理。
 */
#include "prt_hwi.h"
#include "prt_gic_external.h"
#include "prt_gic_internal.h"
#include "prt_attr_external.h"
#include "prt_task.h"
#include "prt_sys_external.h"

#if (OS_GIC_VER == 2)
union IccSgirEl1 {
    struct {
        U32 intId      : 4;
        U32 rsvd0      : 11;
        U32 nsatt      : 1;
        U32 targetlist : 8;
        U32 filter     : 2;
        U32 rsvd1      : 6;
    } bits;
    U32 value;
};
#elif (OS_GIC_VER == 3)
/* ICC_SGIR_EL1 */
union IccSgirEl1 {
    struct {
        U64 targetlist : 16; // bit[0..15] 每bit对应1个核，bit置1代表中断会触发到对应的核
        U64 aff1       : 8; // bit[16..23]
        U64 intId      : 4; // bit[24..27] SGI 中断号.
        U64 rsvd0      : 4; // bit[28..31]
        U64 aff2       : 8; // bit[32..39]
        /* bit[40] 0:中断触发给Aff3.Aff2.Aff1.<target list>;1:中断触发给本核以外的所有核 */
        U64 irm        : 1; // bit[40]
        U64 rsvd1      : 3; // bit[41..43]
        /* bit[44..47] range selector，RS域共4个bit,可以表示16个范围,16×16=256，刚好表示256个cpu */
        /* 所以spec里面，说TargetList[n]表示的aff0的值为RS*16 + n。 */
        U64 rs         : 4; // bit[44..47]
        U64 aff3       : 8; // bit[48..55]
        U64 rsvd2      : 8; // bit[63..56]
    } bits;
    U64 value;
};
#endif

/* GIC基地址 */
OS_SEC_BSS uintptr_t g_gicdBase;
/* GICR相对于GIC基地址偏移向量 */
OS_SEC_BSS uintptr_t g_gicrOffset;
/* GICR核间偏移向量配置 */
OS_SEC_BSS uintptr_t g_gicrStride;
/* 存放Core Map值 */
#if defined(OS_OPTION_SMP)
OS_SEC_DATA union GicCoreMap g_gicCoreMap[OS_MAX_CORE_NUM] = {0};
#else
OS_SEC_DATA union GicCoreMap g_gicCoreMap = {0};
#endif
/*
 * 描述: 去使能指定中断
 */
OS_SEC_L4_TEXT void OsGicDisableInt(U32 intId)
{
    if (intId <= MAX_NNSPI_ID) {
#if defined(OS_OPTION_SMP)
        U32 coreId;

        for (coreId = g_cfgPrimaryCore; coreId < g_cfgPrimaryCore + g_numOfCores; coreId++) {
            OsGicrDisableInt(coreId, intId);
        }
#else
        OsGicrDisableInt(PRT_GetCoreID(), intId);
#endif
    } else if (intId <= MAX_SPI_ID) {
        OsGicdDisableInt(intId);
    } else if (intId >= MIN_LPI_ID) {
        OsGicrLpiDisableInt(intId);
    }
}

/*
 * 描述: 使能指定中断
 */
OS_SEC_L4_TEXT void OsGicEnableInt(U32 intId)
{
    if (intId <= MAX_NNSPI_ID) {
#if defined(OS_OPTION_SMP)
        U32 coreId;

        for (coreId = g_cfgPrimaryCore; coreId < g_cfgPrimaryCore + g_numOfCores; coreId++) {
            OsGicrEnableInt(coreId, intId);
        }
#else
        OsGicrEnableInt(PRT_GetCoreID(), intId);
#endif
    } else if (intId <= MAX_SPI_ID) {
        OsGicdEnableInt(intId);
    } else if (intId >= MIN_LPI_ID) {
        OsGicrLpiEnableInt(intId);
    }
}

/*
 * 描述: 清除指定中断号的pending位
 */
OS_SEC_L4_TEXT void OsGicClearPendingBit(U32 intId)
{
    U32 coreId = OsGetCoreID();
    if (intId <= MAX_NNSPI_ID) {
#if defined(OS_OPTION_SMP)
        for (coreId = g_cfgPrimaryCore; coreId < g_cfgPrimaryCore + g_numOfCores; coreId++) {
            OsGicrClearPendingBit(coreId, intId);
        }
#else
        OsGicrClearPendingBit(coreId, intId);
#endif
    } else if (intId <= MAX_SPI_ID) {
        OsGicdClearPendingBit(intId);
    }
}

/*
 * 描述: 清除指定中断号的Active位
 */
OS_SEC_L4_TEXT void OsGicClearActiveBit(U32 intId)
{
    U32 coreId = OsGetCoreID();
    if (intId <= MAX_NNSPI_ID) {
#if defined(OS_OPTION_SMP)
        for (coreId = g_cfgPrimaryCore; coreId < g_cfgPrimaryCore + g_numOfCores; coreId++) {
            OsGicrClearActiveBit(coreId, intId);
        }
#else
        OsGicrClearActiveBit(coreId, intId);
#endif
    } else if (intId <= MAX_SPI_ID) {
        OsGicdClearActiveBit(intId);
    }
}

#if (OS_GIC_VER == 2)
/*
 * 描述: 触发中断到目标核，仅支持SGI
 */
OS_SEC_TEXT void OsGicTrigIntToCores(U32 intId, U32 targetList)
{
    union IccSgirEl1 iccSgirEl1;
    U32 core;
    
    PRT_DSB();
    for (core = 0; (core < OS_MAX_CORE_NUM) && (targetList != 0); ++core) {
        if ((targetList & (1U << core)) != 0) {
            iccSgirEl1.value           = 0;   // 每个位域默认为0
            iccSgirEl1.bits.intId      = intId;
            iccSgirEl1.bits.targetlist = 1 << core;
            GIC_REG_WRITE(GICD_SGIR, iccSgirEl1.value);
        }
    }
    /* 内存屏障，强制生效执行上述对ICC_SGI1R_EL1的写操作 */
    PRT_ISB();
}
#elif (OS_GIC_VER == 3)
/*
 * 描述: 触发中断到目标核，仅支持SGI
 */
OS_SEC_TEXT void OsGicTrigIntToCores(U32 intId, U32 targetList)
{
    union IccSgirEl1 iccSgirEl1;
    U32 coreId;
#if defined(OS_OPTION_SMP)
    U8 aff0;
#else
    U16 targetMask = 0x1;
#endif
    
    PRT_DSB();
    for (coreId = 0; (coreId < OS_MAX_CORE_NUM) && (targetList != 0); coreId++) {
        if ((targetList & (1U << coreId)) != 0) {
            iccSgirEl1.value           = 0;   // 每个位域默认为0
            iccSgirEl1.bits.intId      = intId;
#if defined(OS_OPTION_SMP)
            aff0                       = g_gicCoreMap[coreId].bits.aff0;
            iccSgirEl1.bits.targetlist = (U16)(1U << OS_GET_8BIT_LOW_4BIT((U32)aff0));
            iccSgirEl1.bits.aff1       = g_gicCoreMap[coreId].bits.aff1;
            iccSgirEl1.bits.aff2       = g_gicCoreMap[coreId].bits.aff2;
            iccSgirEl1.bits.aff3       = g_gicCoreMap[coreId].bits.aff3;
            iccSgirEl1.bits.rs         = OS_GET_8BIT_HIGH_4BIT((U32)aff0);
            targetList &= ~(1U << coreId);
#else
            iccSgirEl1.bits.targetlist = targetMask;
            iccSgirEl1.bits.aff1       = coreId;
            iccSgirEl1.bits.aff2       = g_gicCoreMap.bits.aff2;
            iccSgirEl1.bits.aff3       = g_gicCoreMap.bits.aff3;
#endif
            OS_EMBED_ASM("MSR " REG_ALIAS(ICC_SGI1R_EL1) ", %0 \n" : : "r"(iccSgirEl1.value) : "memory");
        }
    }
    /* 内存屏障，强制生效执行上述对ICC_SGI1R_EL1的写操作 */
    PRT_ISB();
}
#endif


/*
 * 描述: 设置中断的优先级
 */
OS_SEC_L4_TEXT U32 OsGicSetPriority(U32 intId, U32 priority)
{
    U32 coreId;
    enum GicIntState state;
    
    if (intId > MAX_SPI_ID || priority > MAX_INT_PRIORITY) {
        return OS_FAIL;
    }
    
    /* 修改配置前，务必保证中断处于禁能状态 */
    if (intId <= MAX_NNSPI_ID) {
#if defined(OS_OPTION_SMP)
        for (coreId = g_cfgPrimaryCore; coreId < g_cfgPrimaryCore + g_numOfCores; coreId++) {
            state = OsGicrGetIntState(coreId, intId);
            OsGicrDisableInt(coreId, intId);
            OsGicrSetPriority(coreId, intId, priority);
            if (state == GIC_ENABLE) {
                OsGicrEnableInt(coreId, intId);
            }
        }
#else
        coreId = OsGetCoreID();
        state = OsGicrGetIntState(coreId, intId);
        OsGicrDisableInt(coreId, intId);
        OsGicrSetPriority(coreId, intId, priority);
        if (state == GIC_ENABLE) {
            OsGicrEnableInt(coreId, intId);
        }
#endif
    } else {
        state = OsGicdGetIntState(intId);
        OsGicdDisableInt(intId);
        OsGicdSetPriority(intId, priority);
        if (state == GIC_ENABLE) {
            OsGicdEnableInt(intId);
        }
    }
    return OS_OK;
}

/*
 * 描述: 获取中断的优先级
 */
OS_SEC_L4_TEXT U32 OsGicGetPriority(U32 intId)
{
    if (intId <= MAX_NNSPI_ID) {
        return OsGicrGetPriority(PRT_GetCoreID(), intId);
    }

    return OsGicdGetPriority(intId);
}

/*
 * 描述: 设置中断路由目标核，参数有效性由调用者保证
 * 备注: 仅对非N-N SPI有效，仅支持1个目标核。
 */
OS_SEC_L4_TEXT void OsGicSetTargetId(U32 intId, U32 targetId)
{
    enum GicIntState state;

    /* 修改配置前，务必保证中断处于禁能状态 */
    state = OsGicdGetIntState(intId);
    OsGicdDisableInt(intId);
    OsGicdCfgTargetId(intId, targetId);
    if (state == GIC_ENABLE) {
        OsGicdEnableInt(intId);
    }
}

/*
 * 描述: 配置GIC基地址
 * 备注: 此处仅对入参做基础校验，需要用户参考硬件手册，保证入参的正确。
 */
OS_SEC_L4_TEXT U32 OsGicConfigRegister(uintptr_t gicdBase, uintptr_t gicrOffset, uintptr_t gicrStride)
{
    if ((gicdBase == 0) || (gicrOffset == 0) || (gicrStride == 0)) {
        return OS_ERROR_HWI_BASE_ADDR_INVALID;
    }

    g_gicdBase = gicdBase;
    g_gicrOffset = gicrOffset;
    g_gicrStride = gicrStride;

    return OS_OK;
}

/*
 * 描述: SPI中断范围Check
 */
OS_SEC_L4_TEXT bool OsGicIsSpi(U32 intId)
{
    return (((intId) >= MIN_SPI_ID) && ((intId) <= MAX_SPI_ID));
}
