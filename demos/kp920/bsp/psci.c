#include "prt_config.h"
#include "prt_config_internal.h"
#include "../../../src/arch/include/prt_attr_external.h"
#include "../../../src/arch/include/prt_cpu_external.h"
#include "cache_asm.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "openamp_common.h"

#define PSCI_FN_BASE    0x84000000
#define PSCI_FN(val)    (PSCI_FN_BASE + (val))
#define PSCI_64BIT      0x40000000
#define PSCI_FN64_BASE  (PSCI_FN_BASE + PSCI_64BIT)
#define PSCI_FN64(n)    (PSCI_FN64_BASE + (n))

#define PSCI_FN_CPU_OFF PSCI_FN(2)
#define PSCI_FN_CPU_ON PSCI_FN64(3)
#define PSCI_FN_AFFINITY_INFO PSCI_FN64(4)

extern U32 OsArmSmccSmc(U64 a0, U64 a1, U64 a2, U64 a3, U64 a4, U64 a5, U64 a6, U64 a7);

OS_SEC_L4_TEXT U32 OsInvokePsciSmc(U64 functionId, U64 arg0, U64 arg1, U64 arg2)
{
    return OsArmSmccSmc(functionId, arg0, arg1, arg2, 0, 0, 0, 0);
}

extern void PRT_HwiDisableAll(void);

OS_SEC_L4_TEXT U32 OsCpuPowerOff(void)
{
    U32 ret;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    /* 去使能所有中断，防止pending中断遗留，导致下次拉核导致程序跑飞 */
    PRT_HwiDisableAll();
    PRT_HwiDisable(OS_OPENAMP_NOTIFY_HWI_NUM);

    /* 刷L1 ICACHE和DCACHE */
    os_asm_flush_dcache_all();
    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
    os_asm_clean_dcache_all();

    /* 清除中断Active状态 */
    OsHwiClear(OS_HWI_IPI_NO_02);
    OsHwiClear(OS_HWI_IPI_NO_07);
    /* SMC陷入异常 */
    ret = OsInvokePsciSmc(PSCI_FN_CPU_OFF, 0, 0, 0);
    if (ret != OS_OK) {
        return -1;
    }

    PRT_HwiRestore(intSave);
    while(1);

    return 0;
}