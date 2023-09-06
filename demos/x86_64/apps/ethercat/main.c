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

#ifdef POSIX_TESTCASE
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#ifdef LINUX_TESTCASE
void kthreadTest(void);
void schedTest(void);
void waitTest(void);
#endif

#ifdef ETHERCAT_TESTCASE
int ethercat_main(void);
int ethercat_init(void);
void test_ethercat_main();
bool wait_for_slave_scan_complete();
bool wait_for_slave_respond();
#endif

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
U64 g_cpuClock = 0;
char g_modelID[64] = {0};
const TskPrior g_testTskPri = 25;

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

void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif
    printf("test entry\n");
#ifdef ETHERCAT_TESTCASE
    ethercat_init();
    if (!wait_for_slave_respond()) {
        printf("[TEST] no slave responding!");
        return;
    }

    if (!wait_for_slave_scan_complete()) {
        printf("[TEST] slave scan not compete yet!");
        return;
    }
    test_ethercat_main();
#endif
#ifdef LINUX_TESTCASE
    kthreadTest();
    schedTest();
    waitTest();
#endif
#ifdef POSIX_TESTCASE
    Init(0, 0, 0, 0);
#endif
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = g_testTskPri;
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

void HwiTimerIsr(HwiArg arg)
{
    PRT_TickISR();
}

U32 PRT_AppInit(void)
{
    U32 ret;

    PRT_CppSystemInit();
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

extern int __wrap_memcmp(const void *s1, const void *s2, size_t cnt)
{
    const char *t1 = s1;
    const char *t2 = s2;
    int res = 0;

    while (cnt-- > 0) {
        if (*t1 > *t2) {
            res = 1;
            break;
        } else if (*t1 < *t2) {
            res = -1;
            break;
        } else {
            t1++;
            t2++;
        }
    }
    return res;
}

extern char *__wrap_strncpy(char *dest, const char *src, unsigned int num)
{
    char *ret = dest;
    while (num && *src) {
        *dest++ = *src++;
        num--;
    }
    while (num) {
        *dest++ = '\0';
        num--;
    }

    return ret;
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

extern int __wrap_strcmp(const char *str1, const char *str2)
{
    int ret = 0;
    while (!(ret = *(unsigned char *)str1 - *(unsigned char *)str2) && *str1) {
        str1++;
        str2++;
    }
    if (ret < 0) {
        return -1;
    } else if (ret > 0) {
        return 1;
    }
    return 0;
}

extern int __wrap_strncmp(const char *dst, const char *src, int n)
{
    while (--n && (*dst++ == *src++));
    return *dst - *src;
}

extern size_t __wrap_strnlen(const char *str, size_t maxsize)
{
    size_t n;

    for (n = 0; n < maxsize && *str; n++, str++) {
        ;
    }

    return n;
}

extern size_t __wrap_strlen(char *str)
{
    int count = 0;
    while (*str != '\0') {
        count++;
        str++;
    }
    return count;
}

extern void *__wrap_memchr(const void *buf, int c, size_t count)
{
    while (count--) {
        if (*(char *)buf == c) {
            return (void *)buf;
        }
        buf = (char *)buf + 1;
    }
    return NULL;
}

