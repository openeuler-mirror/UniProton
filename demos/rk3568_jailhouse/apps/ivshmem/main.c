#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_tick.h"
#include "ivshmem.h"
#include "ivshmem_demo.h"
#include "cpu_config.h"

#ifdef POSIX_TESTCASE
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#define TEST_TASK_NUM 2

TskHandle g_testTskHandle[TEST_TASK_NUM];
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
struct IvshmemDeviceData dev;
int irqCounter;

void TestTask1()
{
#ifdef POSIX_TESTCASE
    Init(0, 0, 0, 0);
#endif

    while (1) {
        PRT_Printf("TestTask1 run!!! \n");
        PRT_TaskDelay(2000);
    }
}

void TestTask2()
{
    int ret;
    PRT_Printf("TestTask2 enter.\n");
    int bdf = PciDeviceFind(VENDORID, DEVICEID, 0);
    if (bdf == -1) {
        PRT_Printf("IVSHMEM: No PCI devices found .. nothing to do.\n");
    } else {
        PRT_Printf("IVSHMEM: Found device at %02x:%02x.%x\n", bdf >> 8, (bdf >> 3) & 0x1f, bdf & 0x3);
    }

    unsigned int class_rev = PciCfgRead(bdf, 0x8, 4);
    if (class_rev != (PCI_DEV_CLASS_OTHER << 24 |
        JAILHOUSE_SHMEM_PROTO_UNDEFINED << 8)) {
        PRT_Printf("IVSHMEM: class/revision %08x, not supported.\n", class_rev);
    } else {
        PRT_Printf("IVSHMEM: class/revision %08x, supported.\n", class_rev);
    }

    int vndr_cap = PciCapFind(bdf, PCI_CAP_VENDOR);
    if (vndr_cap < 0) {
        PRT_Printf("IVSHMEM ERROR: missing vendor capability, vndr_cap: %d\n", vndr_cap);
    } else {
        PRT_Printf("IVSHMEM: has vendor capability, vndr_cap: %d\n", vndr_cap);
    }

    dev.bdf = bdf;
    ret = DeviceInit(&dev);
    if (ret != OS_OK) {
        PRT_Printf("IVSHMEM: init device fail.\n");
        return;
    }
    MmioWrite32(&dev.registers->intControl, 1);
    MmioWrite32(&dev.registers->state, dev.id + 1);
    dev.rwSection[dev.id] = 0;
    dev.outSection[0] = 0;

    while (1) {
        PRT_Printf("TestTask2 run! \n");
        PRT_TaskDelay(1000);
        IrqSend(&dev);
    }
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // create task1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTask1;
    param.taskPrio = 25;
    param.name = "TestTask1";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle[0]);
    if (ret) {
        return ret;
    }

    // create task2
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTask2;
    param.taskPrio = 30;
    param.name = "TestTask2";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle[1]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;

    ret = PciInit();
    if (ret != OS_OK) {
        return ret;
    }

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
    ret = TestShmemStart();
    if (ret) {
        PRT_Printf("ret error!\n");
        return ret;
    }
    /* 暂不使用uart，先直接写串口寄存器地址 */
    // ret = PRT_PrintfInit();
    // if (ret) {
    //     return ret;
    // }
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
    return ret;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
}

