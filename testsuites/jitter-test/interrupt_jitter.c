#include <float.h>
#include <limits.h>
#include "prt_task.h"
#include "prt_sys.h"
#include "prt_hwi.h"
#include "prt_config.h"
#include "cpu_config.h"

extern U64 g_timerFrequency;
#define TEST_CYCLE g_timerFrequency / OS_TICK_PER_SECOND

#define CYCLE_NUM 500
U32 g_cycle_num = 0;
uintptr_t g_cycle = 0;
uintptr_t g_cycle_cnts[CYCLE_NUM];
U32 g_cfgMask;

void TimerTestIsr(uintptr_t para)
{
    (void)para;
    OS_EMBED_ASM("MRS %0, CNTPCT_EL0" : "=r"(g_cycle)::"memory", "cc");

    g_cfgMask = 0x0;
    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(g_cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTP_TVAL_EL0, %0" : : "r"(TEST_CYCLE) : "memory", "cc");

    g_cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(g_cfgMask) : "memory");
    PRT_ISB();

    if (g_cycle_num < CYCLE_NUM) {
        g_cycle_cnts[g_cycle_num] = g_cycle;
        g_cycle_num++;
    }
}

void interrupt_jitter_test_entry()
{
    U32 ret;
    U32 i;
    uintptr_t cycle_diff;
    float time_diff_us;
    float max_jitter_us = 0;
    float min_jitter_us = FLT_MAX;
    float avg_jitter_us = 0;

    printf("\n===Test interrupt jitter start===\n");
    ret = PRT_HwiDelete(TEST_CLK_INT);
    if (ret != OS_OK) {
        printf("\nPRT_HwiDelete failed, ret is %d\n", ret);
        return;
    }

    ret = PRT_HwiSetAttr(TEST_CLK_INT, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("\nPRT_HwiSetAttr failed, ret is %d\n", ret);
        return;
    }
    
    ret = PRT_HwiCreate(TEST_CLK_INT, (HwiProcFunc)TimerTestIsr, 0);
    if (ret != OS_OK) {
        printf("\nPRT_HwiCreate failed, ret is %d\n", ret);
        return;
    }
    
#if (OS_GIC_VER == 3)
    ret = PRT_HwiEnable(TEST_CLK_INT);
    if (ret != OS_OK) {
        printf("\nPRT_HwiEnable failed, ret is %d\n", ret);
        return;
    }
#endif

    while (g_cycle_num < CYCLE_NUM) {}
    for (i = 1; i < CYCLE_NUM; i++) {
        if (g_cycle_cnts[i] >= g_cycle_cnts[i - 1]) {
            cycle_diff = g_cycle_cnts[i] - g_cycle_cnts[i - 1];
        } else {
            cycle_diff = (ULLONG_MAX - g_cycle_cnts[i - 1]) + g_cycle_cnts[i] + 1;
        }

        cycle_diff -= TEST_CYCLE;
        time_diff_us = (float)cycle_diff / (float)(g_timerFrequency / OS_SYS_US_PER_SECOND);
        max_jitter_us = time_diff_us > max_jitter_us ? time_diff_us : max_jitter_us;
        min_jitter_us = time_diff_us < min_jitter_us ? time_diff_us : min_jitter_us;
        avg_jitter_us += time_diff_us;
    }
    avg_jitter_us /= (CYCLE_NUM - 1);

    printf("\nMax interrupt time jitter is %.2f us\n", max_jitter_us);
    printf("\nMin interrupt time jitter is %.2f us\n", min_jitter_us);
    printf("\nAverage interrupt time jitter is %.2f us\n", avg_jitter_us);
    printf("\n===Test interrupt jitter end===\n");
}
