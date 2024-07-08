#include "securec.h"
#include "prt_tick.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "cpu_config.h"
#include "prt_gic_external.h"
#if defined(OS_OPTION_SMP)
#include "prt_module_external.h"
#endif
#include "prt_config.h"

#if (OS_GIC_VER == 3)


enum SicGroupType {
    SIC_GROUP_G0S  = 0,
    SIC_GROUP_G1NS = 1,
    SIC_GROUP_G1S  = 2,
    SIC_GROUP_BUTT,
};

union SicrWaker {
    struct {
        U32 res0     : 1;
        U32 sleepReq : 1;
        U32 isSleep  : 1;
        U32 res1     : 29;
    } bits;
    U32 value;
};

union SicrCtrl {
    struct {
        U32 enLpis : 1;
        U32 res2   : 2;
        U32 rwp    : 1;
        U32 res1   : 20;
        U32 dpg0   : 1;
        U32 dpg1ns : 1;
        U32 dpg1s  : 1;
        U32 res0   : 4;
        U32 uwp    : 1;
    } bits;
    U32 value;
};

union HwiIccSreElx {
    struct {
        U64 prt    : 1;
        U64 dfb    : 1;
        U64 dib    : 1;
        U64 enable : 1;
        U64 res    : 60;
    } bits;
    U64 value;
};

union SicdCtrl {
    struct {
        U32 enG0S  : 1;
        U32 enG1Ns : 1;
        U32 enG1S  : 1;
        U32 res1   : 1;
        U32 areS   : 1;
        U32 areNs  : 1;
        U32 ds     : 1;
        U32 res0   : 24;
        U32 rwp    : 1;
    } bits;
    U32 value;
};

extern void OsGicdWaitCfgWork();

void OsSicrInit(void)
{
    union SicrWaker sicrWaker;
    uintptr_t regAddr;
    U32 intId;

    regAddr = GICR_WAKER_ADDR + OsGetCoreID() * SICR_ADDR_OFFSET_PER_CORE;
    sicrWaker.value = GIC_REG_READ(regAddr);
    sicrWaker.bits.sleepReq = 0;
    GIC_REG_WRITE(regAddr, sicrWaker.value);
    sicrWaker.value = GIC_REG_READ(regAddr);
    while (sicrWaker.bits.isSleep == 1) {
        sicrWaker.value = GIC_REG_READ(regAddr);
    }
}

void OsSicrSetIntGroup(U32 coreId, U64 intId, enum SicGroupType groupId)
{
    uintptr_t group0RegAddr;
    uintptr_t modRegAddr;
    U32 group0RegTmp;
    U32 modRegTmp;
    
    group0RegAddr = GICR_IGROUPR0_ADDR + OsGetCoreID() * SICR_ADDR_OFFSET_PER_CORE;
    group0RegTmp = GIC_REG_READ(group0RegAddr);
    
    if ((groupId == SIC_GROUP_G0S) || (groupId == SIC_GROUP_G1S)) {
        group0RegTmp &= ~(0x1U << intId);
    } else {
        group0RegTmp |= (0x1U << intId);
    }
    GIC_REG_WRITE(group0RegAddr, group0RegTmp);
    
    modRegAddr = GICR_IGRPMODR0_ADDR + OsGetCoreID() * SICR_ADDR_OFFSET_PER_CORE;
    modRegTmp = GIC_REG_READ(modRegAddr);
    
    if (groupId == SIC_GROUP_G1S) {
        modRegTmp |= (0x1U << intId);
    } else {
        modRegTmp &= ~(0x1U << intId);
    }
    GIC_REG_WRITE(modRegAddr, modRegTmp);
}

U32 OsSiccEnableSre(void)
{
    volatile union HwiIccSreElx iccSre;
    
    OS_EMBED_ASM("MRS %0, " REG_ALIAS(ICC_SRE_EL1) " \n" : "=&r"(iccSre));
    iccSre.bits.prt = 1;
    iccSre.bits.dfb = 1;
    iccSre.bits.dib = 1;
    OS_EMBED_ASM("MSR " REG_ALIAS(ICC_SRE_EL1) ", %0 \n" : : "r"(iccSre));
    OS_EMBED_ASM("DSB SY");
    OS_EMBED_ASM("ISB");

    OS_EMBED_ASM("MRS %0, " REG_ALIAS(ICC_SRE_EL1) " \n" : "=&r"(iccSre.value));
    
    if (iccSre.bits.prt != 1) {
        return OS_FAIL;
    }
    
    return OS_OK;
}

void OsSiccCfgIntPreempt(void)
{
    U64 tmp = 0;
    
    OS_EMBED_ASM("MRS %0, " REG_ALIAS(ICC_BPR1_EL1) " \n" : "=&r"(tmp) : : "memory");
    tmp &= ~(GROUP_MAX_BPR);
    tmp |= GROUP1_BP;
    OS_EMBED_ASM("MSR " REG_ALIAS(ICC_BPR1_EL1) ", %0 \n" : : "r"(tmp) : "memory");
}

void OsSiccEnableGroup1(void)
{
    U64 tmp = 0;
    
    OS_EMBED_ASM("MRS %0, " REG_ALIAS(ICC_IGRPEN1_EL1) " \n" : "=&r"(tmp) : : "memory");
    tmp |= 0x01U;
    OS_EMBED_ASM("MSR " REG_ALIAS(ICC_IGRPEN1_EL1) ", %0 \n" : : "r"(tmp) : "memory");
}

void OsSiccCfgPriorityMask(void)
{
    U64 tmp = 0;
    
    OS_EMBED_ASM("MRS %0, " REG_ALIAS(ICC_PMR_EL1) " \n" : "=&r"(tmp) : : "memory");
    tmp |= PRIO_MASK_LEVEL;
    OS_EMBED_ASM("MSR " REG_ALIAS(ICC_PMR_EL1) ", %0 \n" : : "r"(tmp) : "memory");
}

U32 OsSiccInit(void)
{
    U32 ret;

    ret = OsSiccEnableSre();
    if (ret != OS_OK) {
        return ret;
    }

    OsSiccCfgIntPreempt();

    OsSiccEnableGroup1();

    OsSiccCfgPriorityMask();

    return OS_OK;
}

void OsSicdSetIntGroup(U32 intId, enum SicGroupType groupId)
{
    U64 group0RegAddr;
    U64 modRegAddr;
    U32 group0RegTmp;
    U32 modRegTmp;
    U32 sicdM;
    U32 bitOffset;
    
    sicdM = (intId - MIN_GIC_SPI_NUM) / SICD_IGROUP_INT_NUM;
    group0RegAddr = GICD_IGROUPN_ADDR + (SICD_REG_SIZE * sicdM);
    modRegAddr = GICD_IGRPMODRN_ADDR + (SICD_REG_SIZE * sicdM);
    bitOffset = (intId - MIN_GIC_SPI_NUM) % SICD_IGROUP_INT_NUM;
    
    group0RegTmp = GIC_REG_READ(group0RegAddr);
    if ((groupId == SIC_GROUP_G0S) || (groupId == SIC_GROUP_G1S)) {
        group0RegTmp &= ~(0x1U << bitOffset);
    } else {
        group0RegTmp |= (0x1U << bitOffset);
    }
    GIC_REG_WRITE(group0RegAddr, group0RegTmp);
    
    modRegTmp = GIC_REG_READ(modRegAddr); 
    if (groupId == SIC_GROUP_G1S) {
        modRegTmp |= (0x1U << bitOffset);
    } else {
        modRegTmp &= ~(0x1U << bitOffset);
    }
    GIC_REG_WRITE(modRegAddr, modRegTmp);
}

void OsSicSetGroup(U32 intId, enum SicGroupType groupId)
{
    U32 coreId = OsGetCoreID();
    enum GicIntState state;
    
    if (intId < MIN_GIC_SPI_NUM) {
        state = OsGicrGetIntState(coreId, intId);
        OsGicrDisableInt(coreId, intId);
        OsSicrSetIntGroup(coreId, intId, groupId);
        if (state == GIC_ENABLE) {
            OsGicrEnableInt(coreId, intId);
        }
    } else {
        state = OsGicdGetIntState(intId);
        OsGicdDisableInt(intId);
        OsSicdSetIntGroup(intId, groupId);
        if (state == GIC_ENABLE) {
            OsGicdEnableInt(intId);
        }
    }
}

U32 OsSicInitLocal(void)
{
    U32 ret;
    U32 intId;
    U32 coreID = PRT_GetCoreID();

    OsSicrInit();

    ret = OsSiccInit();
    if (ret != OS_OK) {
        return ret;
    }

    for (intId = 0; intId < MIN_GIC_SPI_NUM; ++intId) {
        OsSicSetGroup(intId, SIC_GROUP_G1NS);
    }

    if (coreID == OS_SYS_CORE_PRIMARY) {
        for (coreID = OS_SYS_CORE_PRIMARY; coreID < OS_SYS_CORE_PRIMARY + OS_SYS_CORE_RUN_NUM; coreID++) {
            /* 清除SGI和PPI的disable，pending，active */
            for (intId = 0; intId < MIN_GIC_SPI_NUM; ++intId) {
                OsGicrDisableInt(coreID, intId);
            }

            for (intId = 0; intId < MIN_GIC_SPI_NUM; ++intId) {
                OsGicrClearPendingBit(coreID, intId);
            }

            for (intId = 0; intId < MIN_GIC_SPI_NUM; ++intId) {
                OsGicrClearActiveBit(coreID, intId);
            }
        }
    }

    return OS_OK;
}

void OsSicdInit(void)
{
    union SicdCtrl sicdCtrl;
    
    sicdCtrl.value = GIC_REG_READ(GICD_CTLR_S_ADDR);
    
    if ((sicdCtrl.bits.enG0S == 1) || (sicdCtrl.bits.enG1Ns == 1) || (sicdCtrl.bits.enG1S == 1)) {
        return;
    }
    
    sicdCtrl.bits.ds = 0;
    sicdCtrl.bits.areNs = 1;
    sicdCtrl.bits.areS = 1;
    sicdCtrl.bits.enG1Ns = 1;
    GIC_REG_WRITE(GICD_CTLR_S_ADDR, sicdCtrl.value);
    
    OsGicdWaitCfgWork();
}

void OsSicInitGlobal(void)
{
    U32 intId;
    
    OsSicdInit();
    
    for (intId = MIN_GIC_SPI_NUM; intId < MAX_INT_NUM; ++intId) {
        OsSicSetGroup(intId, SIC_GROUP_G1NS);
    }
}
#elif (OS_GIC_VER == 2)
int IsrRegister(U32 intNo, U32 pri)
{
    U32 bitn, reg, shift;
    U32 *addr;
    
    bitn = intNo / 32U;
    addr = (U32 *)(GICD_ISENABLERn + 4U * bitn);
    reg = *addr;
    *addr = (reg | (0x1U << (intNo % 32U)));
    
    bitn = intNo / 4U;
    addr = (U32 *)(GICD_IPRIORITYn + 4U * bitn);
    shift = (intNo % 4U) * 8U;
    reg = (*addr) & ~(0xFFU << shift);
    *addr = (reg | pri << shift);
}

void OsGicInitCpuInterface(void)
{
    int i;
    U32 val;

    GIC_REG_WRITE(GICD_ICACTIVERn, 0xFFFFFFFF);
    GIC_REG_WRITE(GICD_ICENABLERn, 0xFFFF0000);
    GIC_REG_WRITE(GICD_ISENABLERn, 0x0000FFFF);

    for (i = 0; i < 32; i += 4) {
        GIC_REG_WRITE(GICD_IPRIORITYn + i, 0xA0A0A0A0);
    }

    GIC_REG_WRITE(GICC_PMR, 0xF0);
    val = GIC_REG_READ(GICC_CTLR);
    val &= ~GICC_CTLR_BYPASS_MASK;
    val |= GICC_CTLR_ENABLE_MASK;
    GIC_REG_WRITE(GICC_CTLR, val);
}
#endif

INIT_SEC_L4_TEXT U32 OsGicInitSecondary(void) {
    OsSicInitLocal();
    return OS_OK;
}

U32 OsHwiInit(void)
{
#if (OS_GIC_VER == 3)
    U32 ret;

    ret = OsSicInitLocal();
    if (ret != OS_OK) {
        return ret;
    }

#if defined(OS_OPTION_SMP)
    OsHwiSetSecondaryInitHook(OsGicInitSecondary);
#endif

    if(PRT_GetCoreID() == 0) {
        OsSicInitGlobal();
    }
#elif (OS_GIC_VER == 2)
    OsGicInitCpuInterface();
#endif
    return OS_OK;
}
