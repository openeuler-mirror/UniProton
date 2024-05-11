#include "prt_cpu_external.h"
#include "prt_attr_external.h"
#include "prt_amp_psci_internal.h"

OS_SEC_L4_TEXT U32 OsInvokePsciSmc(U64 functionId, U64 arg0, U64 arg1, U64 arg2)
{
    return OsArmSmccSmc(functionId, arg0, arg1, arg2, 0, 0, 0, 0);
}
OS_SEC_TEXT void OsCpuPowerOff(void)
{
    U32 ret;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    OsHwiDisableAll();

#ifdef OS_OPTION_OPENAMP
    if (g_setOfflineFlagHook != NULL) {
        g_setOfflineFlagHook();
    }
#endif

    /* 刷L1 ICACHE和DCACHE */
    os_asm_flush_dcache_all();
    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
    os_asm_clean_dcache_all();

    /* 清除中断Active状态 */
    OsHwiClear(OS_HWI_IPI_NO_02); /* 基于中断的power off */
    OsHwiClear(OS_HWI_IPI_NO_07); /* 基于消息的power off */

    /* SMC陷入异常 */
    (void)OsInvokePsciSmc(PSCI_FN_CPU_OFF, 0, 0, 0);

    /* 正常offline的话不会来到这里 */
    PRT_HwiRestore(intSave);
    while (1);
}