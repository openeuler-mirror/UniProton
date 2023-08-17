#ifndef _PRT_LAPIC_H_
#define _PRT_LAPIC_H_

typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
} CpuInfo;

typedef union {
    U64 icr;
    struct {
        U64 vector : 8;
        U64 deliveryMode : 3;
        U64 destMode : 1;
        U64 reserve0 : 2;
        U64 level : 1;
        U64 triggerMode : 1;
        U64 reserve1 : 2;
        U64 destShort : 2;
        U64 reserve2 : 12;
        U64 destField : 32;
    } info;
} IcrInfo;

#define OS_SYSTICK_CONTROL_COUNTFLAG_MSK (1U << 16)
#define MSR_APICBASE          0x1b
#define MSR_APICBASE_ENABLE   (1<<11)
#define MSR_X2APICBASE_ENABLE (1<<10)

#define CPUID_GETFEATURES 1

#define CPUID_EDX_APIC   (1 << 9)
#define CPUID_ECX_x2APIC (1 << 21)

#define X2APIC_SVR       0x80f
#define X2APIC_TDCR      0x83e
#define LAPIC_LVT_TIMER  0x832
#define X2APIC_TICR      0x838
#define X2APIC_TCCR      0x839
#define X2APIC_LVT_LINT0 0x835
#define X2APIC_LVT_LINT1 0x836
#define X2APIC_LVT_TSR   0x833
#define X2APIC_LVT_PMR   0x834
#define X2APIC_LVT_ERROR 0x837
#define X2APIC_ESR       0x828
#define X2APIC_ICR       0x830
#define X2APIC_EOI       0x80b
#define X2APIC_TPR       0x808
#define X2APIC_LID       0x802
#define X2APIC_LDVR      0x803
#define X2APIC_PPR       0x80A
#define X2APIC_LDR       0x80d

#define OS_LAPIC_TIMER      0xfc

extern void OsWriteMsr(U32 msr, U64 value);
extern void OsReadCpuInfo(U32 id, CpuInfo *info);
extern U32 OsLapicInit(void);
extern void OsLapicConfigTick(void);
extern void OsReadMsr(U32 id, U64 *info);

#endif