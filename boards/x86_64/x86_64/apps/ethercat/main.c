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
#include "prt_lapic.h"

#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shell.h"
#include "show.h"
#endif

#if defined(LWIP_DEMO) && defined(OS_SUPPORT_NET)
#include "i210.h"
#endif

#if defined(POSIX_TESTCASE) || defined(RHEALSTONE_TESTCASE) || defined(CXX_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(OS_OPTION_LINUX) && defined(LINUX_TESTCASE)
void kthreadTest(void);
void schedTest(void);
void waitTest(void);
#endif

#if defined(OS_SUPPORT_IGH_ETHERCAT)
int ethercat_init(void);
void ecrt_i210_nic_reg(void);
#endif

#if defined(OS_SUPPORT_IGH_ETHERCAT) && defined(ETHERCAT_TESTCASE)
int ethercat_main(void);
void test_ethercat_main();
bool wait_for_slave_scan_complete();
bool wait_for_slave_respond();
#endif

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
U64 g_cpuClock = 0;
char g_modelID[64] = {0};
const TskPrior g_testTskPri = 25;

#if defined(OS_OPTION_MODBUS)
int modbus_client();
#endif

#if defined(OS_OPTION_FORTE)
void forte_init();
#endif

#if defined(LWIP_DEMO) && defined(OS_SUPPORT_NET)
void lwip_test_start(void);
#endif

#if defined(OS_OPTION_OPENAMP)
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
    ShellCB *shellCb = OsGetShellCB();
    if (ret != 0 || shellCb == NULL) {
        printf("shell init fail\n");
        return;
    }
    (void)memset_s(shellCb->shellBuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    ret = osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
    if (ret == 0) {
        printf("[INFO]: reg cmd 'tstreg' successed!\n");
    } else {
        printf("[INFO]: reg cmd 'tstreg' failed!\n");
    }
}
#endif

#if defined(OS_SUPPORT_LIBXML2) && defined(LIBXML2_TESTCASE)
int xml2_test_entry();
#endif

#if defined(LIBCCL_TESTCASE)
extern void test_ccl();
#endif

void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif
    printf("test entry\n");

#if defined(LWIP_DEMO) && defined(OS_SUPPORT_NET)
    lwip_test_start();
#endif

#if defined(FORTE_DEMO) && defined(OS_OPTION_FORTE)
    forte_init();
#endif

#if defined(MODBUS_DEMO) && defined(OS_OPTION_MODBUS)
    modbus_client();
#endif

#ifdef LOSCFG_SHELL_MICA_INPUT
    micaShellInit();
#endif

#if defined(OS_SUPPORT_IGH_ETHERCAT)
    ecrt_i210_nic_reg();
    ethercat_init();
#endif

#if defined(OS_SUPPORT_IGH_ETHERCAT) && defined(ETHERCAT_TESTCASE)
    if (!wait_for_slave_respond()) {
        printf("[TEST] no slave responding!\n");
        return;
    }

    if (!wait_for_slave_scan_complete()) {
        printf("[TEST] slave scan not compete yet!\n");
        return;
    }
    printf("[TEST] detect slave\n");
    test_ethercat_main();
#endif

#if defined(OS_SUPPORT_LIBXML2) && defined(LIBXML2_TESTCASE)
    xml2_test_entry();
#endif

#if defined(OS_OPTION_LINUX) && defined(LINUX_TESTCASE)
    kthreadTest();
    schedTest();
    waitTest();
#endif
#if defined(POSIX_TESTCASE) || defined(RHEALSTONE_TESTCASE) || defined(CXX_TESTCASE)
    Init(0, 0, 0, 0);
#endif
#if defined(LIBCCL_TESTCASE)
    test_ccl();
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
    return OsConfigStart();
}
