#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "test.h"
#include "rpmsg_backend.h"

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

#if defined(OS_OPTION_OPENAMP)
extern U32 RpmsgHwiInit(void);

int TestOpenamp()
{
    int ret;

    ret = rpmsg_service_init();
    if (ret) {
        return ret;
    }

    return OS_OK;
}
#endif

void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 25;
    param.name = "TestTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;

#if defined(OS_OPTION_OPENAMP)
    /*
     * Linux will send an interrupt to Uniproton after initialising vdev.
     * However, if Uniproton has not registered the corresponding IPI handler,
     * it will throw an exception (call OsHwiDefaultHandler()).
     *
     * And, the Raspberry Pi 4B doesn't support writing the Clear-enable bit (GICD_ICENABLER)
     * for sgi. Therefore, even though we may not need to handle the interrupts sent by Linux
     * until the rpmsg backend is initialised, we also need to register the IPI handler
     * before irq_enable.
     * We will register the actual interrupt handler when rpmsg is initialised.
     */
    ret = RpmsgHwiInit();
    if (ret) {
        return ret;
    }
#endif

    ret = OsTestInit();
    if (ret) {
        return ret;
    }

    ret = TestClkStart();
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret) {
        return ret;
    }

    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

S32 main(void)
{
    return OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL || len == 0) {
        return NULL;
    }

    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i) {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}

