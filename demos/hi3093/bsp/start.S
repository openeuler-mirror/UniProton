#include "prt_buildef.h"
#include "prt_asm_arm_external.h"

    .global   OsResetVector
    .global   OsArmSmccSmc
    .global   mmu_init
    
    .type     mmu_init, function
    .type     start, function
    .section  .text.bspinit, "ax"
    .balign   4
    
#define HCR_EL2_FMO         (1 << 3)
#define HCR_EL2_IMO         (1 << 4)
#define HCR_EL2_AMO         (1 << 5)
#define HCR_EL2_TWI         (1 << 13)
#define HCR_EL2_TWE         (1 << 14)
#define HCR_EL2_TVM         (1 << 26)
#define HCR_EL2_TGE         (1 << 27)
#define HCR_EL2_TDZ         (1 << 28)
#define HCR_EL2_HCD         (1 << 29)
#define HCR_EL2_TRVM        (1 << 30)
#define HCR_EL2_RW          (1 << 31)

#define SPSR_DBG_MASK       (1 << 9)
#define SPSR_SERR_MASK      (1 << 8)
#define SPSR_IRQ_MASK       (1 << 7)
#define SPSR_FIQ_MASK       (1 << 6)
#define SPSR_M_AARCH64      (0 << 4)
#define SPSR_M_AARCH32      (1 << 4)
#define SPSR_M_EL1H         (5)
#define SPSR_M_EL2H         (9)

#define CNTHCTL_EL2_EL1PCEN_EN  (1 << 1)
#define CNTHCTL_EL2_EL1PCTEN_EN (1 << 0)
#define CPACR_EL1_FPEN_EN       (3 << 20)

    .global OsElxState
    .type   OsElxState, @function
OsElxState:
    MRS    x6, CurrentEL
    MOV    x2, #0x4
    CMP    w6, w2
    
    BEQ Start
    
OsEl2Entry:
    MRS    x10, CNTHCTL_EL2
    ORR    x10, x10, #0x3
    MSR    CNTHCTL_EL2, x10

    MRS    x10, CNTKCTL_EL1
    ORR    x10, x10, #0x3
    MSR    CNTKCTL_EL1, x10

    MRS    x10, MIDR_EL1
    MRS    x1, MPIDR_EL1
    MSR    VPIDR_EL2, x10
    MSR    VMPIDR_EL2, x1
    
    MOV    x10, #0x33ff
    MSR    CPTR_EL2, x10
    MSR    HSTR_EL2, xzr

    MRS    x10, CPACR_EL1
    MOV    x10, #3 << 20
    MSR    CPACR_EL1, x10

    MOV    x10, #(HCR_EL2_RW)
    ORR    x10, x10, #(HCR_EL2_HCD)
    BIC    x10, x10, #(HCR_EL2_TVM)
    BIC    x10, x10, #(HCR_EL2_TRVM)
    BIC    x10, x10, #(HCR_EL2_TGE)
    BIC    x10, x10, #(HCR_EL2_AMO)
    BIC    x10, x10, #(HCR_EL2_IMO)
    BIC    x10, x10, #(HCR_EL2_FMO)
    BIC    x10, x10, #(HCR_EL2_TWI)
    BIC    x10, x10, #(HCR_EL2_TWE)
    
    MSR    HCR_EL2, x10
    
OsEl2SwitchToEl1:
    ADR    x0, Start
    MSR    SP_EL1, XZR
    MSR    ELR_EL2, x0
    MOV    x0, XZR
    
    LDR    x20, =(SPSR_DBG_MASK | SPSR_SERR_MASK | \
                  SPSR_IRQ_MASK | SPSR_FIQ_MASK | SPSR_M_EL1H)
    MSR    SPSR_EL2, x20
    
    TLBI   ALLE1IS
    IC     IALLU
    DSB    SY
    ISB
    ERET
    
Start:
#if defined(OS_OPTION_SMP)
    OsAsmGetCoreId  x0 // 读取核号
    LDR x4, =g_cfgPrimaryCore
    LDR w4, [x4]
    CMP w0, w4
    BNE OsSlaveCoreProcess
    
#endif
    LDR    x1, =__os_sys_sp_end
    BIC    sp, x1, #0xf

    BL     mmu_init

#if !defined(OS_OPTION_SMP)
    B      OsResetVector
#endif
    BL    OsSetValidAllCoresMask
    B     OsMasterCoreProcess

OsSlaveCoreProcess:
    OsAsmGetCoreId  x0 // 读取核号
    MOV x2, #0x1000
    mul x0, x0, x2
    LDR x1, =__os_sys_sp_end
    SUB x1, x1, x0
    BIC sp, x1, #0xf

OsSlaveStart:
#ifdef OS_OPTION_OPENAMP
    BL     mmu_init
#endif

    B      OsResetVector

OsMasterCoreProcess:
    B      OsResetVector

OsEnterReset:
    B      OsEnterReset

    .section .text, "ax"
    .balign 4

OsArmSmccSmc:
    smc     #0x0
    ret