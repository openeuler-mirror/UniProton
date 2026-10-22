#include "prt_sys.h"
#include "prt_tick.h"
#include "prt_config.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "securec.h"

U64 g_timerFrequency;
#define PMU_TIMER_FREQUENCY g_timerFrequency

U64 GetGenericTimerFreq(void)
{
    /* Not CNTFRQ_EL0: the firmware sets it wrong on this board, see SYS_COUNTER_FREQ */
    return SYS_COUNTER_FREQ;
}
#if defined(GUEST_OS)
void TimerIsr(uintptr_t para)
{
    (void)para;
    U32 cfgMask = 0x0;
    U64 cycle = PMU_TIMER_FREQUENCY / OS_TICK_PER_SECOND;

    OS_EMBED_ASM("MSR CNTV_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTV_TVAL_EL0, %0" : : "r"(cycle) : "memory", "cc");

    cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTV_CTL_EL0, %0" : : "r"(cfgMask) : "memory");

    PRT_TickISR();
    PRT_ISB();
}

void CoreTimerInit(void)
{
    U32 cfgMask = 0x0;
    U64 cycle = PMU_TIMER_FREQUENCY / OS_TICK_PER_SECOND;

    OS_EMBED_ASM("MSR CNTV_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTV_TVAL_EL0, %0" : : "r"(cycle) : "memory", "cc");

    cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTV_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
}
#else
void TimerIsr(uintptr_t para)
{
    (void)para;
    U64 cycle = PMU_TIMER_FREQUENCY / OS_TICK_PER_SECOND;
    U64 cval;
    U64 now;

    /* Absolute grid: next deadline = previous deadline + one period. Rearming
     * TVAL from ISR entry added the interrupt latency to every tick (999.63 Hz
     * on board). If more than a period behind, realign to now instead of
     * replaying missed ticks. Writing a future CVAL deasserts the timer, so
     * CTL needs no toggling. */
    OS_EMBED_ASM("MRS %0, CNTP_CVAL_EL0" : "=r"(cval) : : "memory");
    OS_EMBED_ASM("MRS %0, CNTPCT_EL0" : "=r"(now) : : "memory");
    cval += cycle;
    if (cval <= now) {
        cval = now + cycle;
    }
    OS_EMBED_ASM("MSR CNTP_CVAL_EL0, %0" : : "r"(cval) : "memory", "cc");
    PRT_ISB();

    PRT_TickISR();
    PRT_ISB();
}

void CoreTimerInit(void)
{
    U32 cfgMask = 0x0;
    U64 cycle = PMU_TIMER_FREQUENCY / OS_TICK_PER_SECOND;

    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTP_TVAL_EL0, %0" : : "r"(cycle) : "memory", "cc");

    cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
}
#endif

U32 CoreTimerStart(void)
{
    g_timerFrequency = GetGenericTimerFreq();
    CoreTimerInit();
    return OS_OK;
}

U32 TestClkStart(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(TEST_CLK_INT, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(TEST_CLK_INT, (HwiProcFunc)TimerIsr, 0);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiEnable(TEST_CLK_INT);
    if (ret != OS_OK) {
        return ret;
    }
    ret = CoreTimerStart();
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

#if defined(OS_OPTION_SMP)
U32 OsCoreTimerSecondaryInit(void)
{
    U32 ret;
    ret = PRT_HwiEnable(TEST_CLK_INT);
    if (ret != OS_OK) {
        return ret;
    }
    ret = CoreTimerStart();
    if (ret != OS_OK) {
        return ret;
    }
    return ret;
}
#endif