#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_log.h"
#include "prt_clk.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "securec.h"
#include "time.h"
#include "kern_test_public.h"
#include "prt_proxy_ext.h"
#define dprintf(format, ...) PRT_ProxyPrintf(format, ##__VA_ARGS__)                     

#if !defined(OS_OPTION_SMP)
extern void OsHwiMcTrigger(U32 coreMask, U32 hwiNum);
#else
extern void OsHwiMcTrigger(enum OsHwiIpiType type, U32 coreMask, U32 hwiNum);
#endif

static void test_interrupt_handler(void)
{
    volatile U32 list[17000] = {0}; // 创建大于系统栈大小的数组，中断栈溢出。
    list[100] = 1;
    TEST_LOG("[SUCCESS] reccceived test interrupt handler.\n");
}

static int test_interrupt_create(U32 hwirq, void *handler_function)
{
    U32 ret = PRT_HwiSetAttr(hwirq, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiCreate(hwirq, (HwiProcFunc)handler_function, 0);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiEnable(hwirq);
    if (ret != OS_OK) {
        return ret;
    }
    dprintf("[SUCCESS] test_interrupt_create.\n");
    return OS_OK;
}


static int test_interrupt_protection(void)
{
    U32 loop;
    test_interrupt_create(OS_HWI_IPI_NO_015, test_interrupt_handler);

#if !defined(OS_OPTION_SMP)
    OsHwiMcTrigger(0xf, OS_HWI_IPI_NO_015);
#else
    OsHwiMcTrigger(OS_TYPE_TRIGGER_TO_SELF, 0, OS_HWI_IPI_NO_015);
#endif

    return 0;
}

test_case_t g_cases[] = {
    TEST_CASE_Y(test_interrupt_protection),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    TEST_LOG("interrupt protection check test finished\n");
}