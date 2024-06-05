#include "prt_typedef.h"
#include "prt_lapic.h"
#include <stdio.h>

#define LAPIC_LVT_TIMER_PERIODIC      (1 << 17)

void OsCycleInit(void);

void OsReadCpuInfo(U32 id, CpuInfo *info)
{
    __asm__ volatile("cpuid" : "=a"(info->eax), "=b"(info->ebx), "=c"(info->ecx), "=d"(info->edx) : "a"(id));
    return;
}

void OsReadMsr(U32 id, U64 *info)
{
    U32 eax;
    U32 edx;
    __asm__ volatile("rdmsr\n" : "=a"(eax), "=d"(edx) : "c"(id));
    *info = (U64)edx << 32 | eax;
    return;
}

void OsWriteMsr(U32 msr, U64 value)
{
    __asm__ volatile("wrmsr\n"
                 :: "c" (msr),
                 "a" ((U32) (value)),
                 "d" ((U32) (value >> 32)));
	return;
}

bool OsSupportApic(void)
{
    CpuInfo info;

    OsReadCpuInfo(CPUID_GETFEATURES, &info);
    if (info.edx & CPUID_EDX_APIC) {
        return TRUE;
    }

    return FALSE;
}

bool OsSupportX2Apic(void)
{
    CpuInfo info;

    OsReadCpuInfo(CPUID_GETFEATURES, &info);
    if (info.ecx & CPUID_ECX_x2APIC) {
        return TRUE;
    }

    return FALSE;
}

U32 OsLapicInit(void)
{
    U64 msrValue;

    OsCycleInit();

    if (OsSupportApic() == FALSE) {
        printf(" CPU not support apic ");
        return OS_FAIL;
    }

    if (OsSupportX2Apic() == FALSE) {
        printf(" CPU not support x2apic ");
        return OS_FAIL;
    }

    /* 使能X2APIC模式 */
    OsReadMsr(MSR_APICBASE, &msrValue);
    msrValue |= MSR_APICBASE_ENABLE;
    msrValue |= MSR_X2APICBASE_ENABLE;
    OsWriteMsr(MSR_APICBASE, msrValue);

    OsWriteMsr(X2APIC_SVR, 0x1ff);
    /* Divide Configuration Register，0xb表示divide by 1 */
    OsWriteMsr(X2APIC_TDCR, 0xb);

    OsWriteMsr(X2APIC_LVT_LINT0, 0x10000);
    OsWriteMsr(X2APIC_LVT_LINT1, 0x10000);
    OsWriteMsr(X2APIC_LVT_PMR, 0x10000);

    OsWriteMsr(X2APIC_LVT_ERROR, 0x33);
    /* Error Status Register，清除寄存器残留错误信息 */
    OsWriteMsr(X2APIC_ESR, 0);
    OsWriteMsr(X2APIC_ESR, 0);

    /* 使能中断功能 */
    OsWriteMsr(X2APIC_EOI, 0);
    /* Task Priority Register，若要触发中断，设置的中断号数据要大于此优先级 */
    OsWriteMsr(X2APIC_TPR, 0xa0);

    return OS_OK;
}

void OsLapicConfigTick(void)
{
    /* Initial Count register，此值会赋值给CCR，当CCR减少到0时触发一次时钟中断 */
    OsWriteMsr(X2APIC_TICR, 25000000 / 8000);
    /* LVT Timer register，设置周期定时器 */
    OsWriteMsr(LAPIC_LVT_TIMER, OS_LAPIC_TIMER | LAPIC_LVT_TIMER_PERIODIC);

    return;
}

void OsTrigerHwi(U32 hwiNum)
{
    IcrInfo icr;
    icr.info.destField = 0xa;
    icr.info.destShort = 0x1;
    icr.info.triggerMode = 0;
    icr.info.level = 1;
    icr.info.destMode = 0;
    icr.info.deliveryMode = 0;
    icr.info.vector = hwiNum;

    OsWriteMsr(X2APIC_ICR, icr.icr);
    return;
}

void OsTriggerHwi(U32 hwiNum, U32 targetCore, U32 shortHand)
{
    IcrInfo icr;
    icr.info.destField = targetCore;
    icr.info.destShort = shortHand; // 0为默认， 1为仅发给自己
    icr.info.triggerMode = 0;
    icr.info.level = 1;
    icr.info.destMode = 0;
    icr.info.deliveryMode = 0;
    icr.info.vector = hwiNum;

    OsWriteMsr(X2APIC_ICR, icr.icr);
    return;
}