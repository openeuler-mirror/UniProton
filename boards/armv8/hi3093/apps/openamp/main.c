#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "prt_log.h"
#include "test.h"
#include "rpmsg_backend.h"
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shell.h"
#include "show.h"
#endif
TskHandle g_sampleHandle[2];
TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

extern U32 PRT_PrintfInit();

#if defined(POSIX_TESTCASE) || defined(CXX_TESTCASE) || defined(EIGEN_TESTCASE) || defined(RHEALSTONE_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(OS_OPTION_OPENAMP)
unsigned int is_tty_ready(void);
#endif

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

#ifdef LOSCFG_SHELL_MICA_INPUT
static int osShellCmdTstReg(int argc, const char **argv)
{
    printf("tstreg: get %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("    no %d arguments: %s\n", i + 1, argv[i]);
    }

    return 0;
}

void micaShellInit()
{
    int ret = OsShellInit(0);
    ShellCB *shellCB = OsGetShellCB();
    if (ret != 0 || shellCB == NULL) {
        PRT_Printf("shell init fail\n");
        return;
    }

    (VOID)memset_s(shellCB->shellBuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    ret = osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
    if (ret == 0) {
        PRT_Printf("[INFO]: reg cmd 'tstreg' successed!\n");
    } else {
        PRT_Printf("[INFO]: reg cmd 'tstreg' failed!\n");
    }
}
#endif

#if defined(OS_SUPPORT_LIBXML2) && defined(LIBXML2_TESTCASE)
int xml2_test_entry();
#endif

#if defined(SOEM_DEMO) && defined(OS_SUPPORT_SOEM)
void soem_test(const char *ifname);
#endif

#if defined(LIBCCL_TESTCASE)
extern void test_ccl();
#endif

void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif

#ifdef LOSCFG_SHELL_MICA_INPUT
    micaShellInit();
#endif

#if defined(OS_OPTION_OPENAMP)
    while (!is_tty_ready()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
#endif

#if defined(SOEM_DEMO) && defined(OS_SUPPORT_SOEM)
    soem_test("eth3");
#endif

#if defined(POSIX_TESTCASE) || defined(CXX_TESTCASE) || defined(EIGEN_TESTCASE) || defined(RHEALSTONE_TESTCASE)
    PRT_Printf("ENTER INIT\n");
    Init(0, 0, 0, 0);
#endif

#if defined(OS_SUPPORT_LIBXML2) && defined(LIBXML2_TESTCASE)
    xml2_test_entry();
#endif

#if defined(LIBCCL_TESTCASE)
    test_ccl();
#endif
}

#if defined(DRIVER_TESTCASE)
extern int app_main(void);
void DriverSampleEntry()
{
    PRT_Printf("DriveSampleEntry\n");
    app_main();
}
#endif

void Test2TaskEntry()
{
    while (1) {
        PRT_Printf("task 1.\n");
        PRT_TaskDelay(6000);
    }
}

void Test3TaskEntry()
{
    while (1) {
        PRT_Printf("task 2.\n");
        PRT_TaskDelay(4000);
    }
}

U32 TaskTest()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];

    // task 2
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Test2TaskEntry;
    param.taskPrio = 20;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }

    // task 3
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Test3TaskEntry;
    param.taskPrio = 25;
    param.name = "Test3Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[1]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 25;
    param.name = "TestTask";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret) {
        return ret;
    }
    
    ret = PRT_TaskResume(g_testTskHandle);
    if (ret) {
        return ret;
    }

#if defined(DRIVER_TESTCASE)
    // DriverSampleEntry
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)DriverSampleEntry;
    param.taskPrio = 26;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_sampleHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskCoreBind(g_sampleHandle[0], 1 << 3);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_sampleHandle[0]);
    if (ret) {
        return ret;
    }
#endif

    return OS_OK;
}

#if defined(OS_OPTION_SMP) && (LOG_TESTCASE)
static void masterLogTask(void)
{
    PRT_Printf("mst enter\n");
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "mst enter", 9);
    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "mst log init", 12);
    while (1) {
        PRT_TaskDelay(OS_TICK_PER_SECOND * 10);
        (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "mst log", 7);
    }
}

static U32 logTestInit(void)
{
    U32 ret;
    TskHandle testTskHandle;
    struct TskInitParam param = {0};

    if (OsGetCoreID() != OS_SYS_CORE_PRIMARY) {
        return 0;
    }
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)masterLogTask;
    // 支持的优先级(0~31)
    param.taskPrio = 24;
    param.name = "masterlog";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        return OS_FAIL;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        return OS_FAIL;
    }
    return ret;
    }
#endif
#if (SMP_TESTCASE)
void SlaveTaskEntry()
{
    PRT_Printf("slave 1.\n");
    static U32 temp1 = 0;
    while (1) {
        PRT_TaskDelay(5000);
        PRT_Printf("slave 1. %d\n", OsGetCoreID());
        temp1++;
    }
}

void SlaveTaskEntry2()
{
    static U32 temp2 = 0;
    while (1) {
        PRT_Printf("slave 2. %d\n", OsGetCoreID());
        PRT_TaskDelay(3000);
        temp2++;
    }
}

U32 SlaveTestInit(U32 slaveId)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle[2];
    // task 1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry;
    param.taskPrio = 25;
    param.name = "SlaveTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskCoreBind(testTskHandle[0], 1 << (PRT_GetPrimaryCore() + slaveId));
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[0]);
    if (ret) {
        return ret;
    }
    
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)SlaveTaskEntry2;
    param.taskPrio = 30;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskCoreBind(testTskHandle[1], 1 << (PRT_GetPrimaryCore() + slaveId));
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle[1]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}
#endif

U32 PRT_AppInit(void)
{
    U32 ret;
#ifdef OS_SUPPORT_CXX
    PRT_CppSystemInit();
#endif

#if defined(OS_OPTION_OPENAMP)
    /*
     * Linux will send an interrupt to Uniproton after initialising vdev.
     * However, if Uniproton has not registered the corresponding IPI handler,
     * it will throw an exception (call OsHwiDefaultHandler()).
     *
     * Therefore, even though we may not need to handle the interrupts sent by Linux
     * until the rpmsg backend is initialised, we also need to register the IPI handler
     * before irq_enable.
     * We will register the actual interrupt handler when rpmsg is initialised.
     */
    ret = RpmsgHwiInit();
    if (ret) {
        return ret;
    }

#if defined(OS_OPTION_POWEROFF)
    OsSetOfflineFlagHook(rsc_table_set_offline_flag);
#endif

#endif
    
#if defined(OS_OPTION_SMP)
#if (SMP_TESTCASE)
    ret =TaskTest();
    if (ret) {
        return ret;
    }

    for (U32 slaveId = 1; slaveId < OS_SYS_CORE_RUN_NUM; slaveId++) {
        ret = SlaveTestInit(slaveId);
        if (ret) {
            return ret;
        }
    }
#endif
#if (LOG_TESTCASE)
    ret = logTestInit();
    if (ret) {
        return OS_FAIL;
    }
#endif
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

    ret = PRT_PrintfInit();
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

