#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_lapic.h"

#include "i40ecs.h"

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
U64 g_cpuClock = 0;
char g_modelID[64] = {0};
const TskPrior g_testTskPri = 25;

#if defined(OS_OPTION_OPENAMP)
int TestFileProxy()
{
    char buf[1024] = {'\0'};
    printf("ready to open file\n");
    int fd = open("test.log", O_RDWR|O_CREAT|O_APPEND, S_IRWXU);
    printf("get fd: %d\n", fd);
    if (fd == -1) {
        return fd;
    }
    int ret = close(fd);
    return ret;
}

int TestOpenamp()
{
    int ret;

    ret = rpmsg_service_init();
    if (ret) {
        return ret;
    }

    printf("ready to start test fileProxy!\n");
    ret = TestFileProxy();
    if (ret) {
        return ret;
    }
    return 0;
}
#endif

void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif
    printf("openamp test entry\n");

#if defined(OS_SUPPORT_I40E)
    i40e_init();
    i40ecs_test_start();
#endif
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x9000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = g_testTskPri;
    param.name = "TestTask";
    param.stackSize = 0x9000;
    
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

void HwiTimerIsr(HwiArg arg)
{
    PRT_TickISR();
}

U32 PRT_AppInit(void)
{
    U32 ret;
#ifdef OS_SUPPORT_CXX
    PRT_CppSystemInit();
#endif
    ret = OsTestInit();
    if (ret) {
        return ret;
    }
    return OS_OK;
}

static void CpuId(unsigned int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    *eax = op;
    *ecx = 0;
    OS_EMBED_ASM(
          "cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx)
        : "memory"
    );
}

static void GetCpuName(char *modelID)
{
    U32 *v = (U32 *)modelID;
    CpuId(0x80000002, &v[0], &v[1], &v[2], &v[3]);
    CpuId(0x80000003, &v[4], &v[5], &v[6], &v[7]);
    CpuId(0x80000004, &v[8], &v[9], &v[10], &v[11]);
    modelID[48] = 0;
}

static void GetCpuCycle()
{
    char cycleArr[10] = {0};
    U32 i;
    GetCpuName(g_modelID);

    for (i = 0; i < sizeof(g_modelID) && g_modelID[i] != '\0'; i++) {
        if (g_modelID[i] == '@') {
            g_cpuClock += (U64)(g_modelID[i + 2] - '0') * 1e9;
            g_cpuClock += (U64)(g_modelID[i + 4] - '0') * 1e8;
            g_cpuClock += (U64)(g_modelID[i + 5] - '0') * 1e7;
            break;
        }
    }
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    GetCpuCycle();

    ret = PRT_HwiSetAttr(OS_LAPIC_TIMER, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("line %d, errno: 0x%08x\n", __LINE__, ret);
        return ret;
    }

    ret = PRT_HwiCreate(OS_LAPIC_TIMER, (HwiProcFunc)HwiTimerIsr, 0);
    if (ret != OS_OK) {
        printf("create hwi failed:0x%llx\n", ret);
        return ret;
    }

    OsLapicConfigTick();

    CpuOfflineHwiCreate();

    return OS_OK;
}

S32 main(void)
{
    PrintInit();
    return OsConfigStart();
}
