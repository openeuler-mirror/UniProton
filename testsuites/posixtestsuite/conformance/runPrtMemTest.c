#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_module.h"
#include "prt_sem.h"
#include "runPrtMemTest.h"

/* common 用例统一经标准 PRT_Mem* 接口操作默认分区，mid=OS_MID_MEM，ptNo=默认分区。
   不引用任何算法私有符号/布局，故 FSC 与 TLSF 均应通过。 */

#define PRT_MEM_PT OS_MEM_DEFAULT_FSC_PT
#define PRT_MEM_MID OS_MID_MEM

static volatile int g_prtMemTaskDone;
static volatile int g_prtMemTaskResult;

/* prt_mem_001：基本分配/写入校验/释放。 */
int prt_mem_001(void)
{
    uint8_t *p = (uint8_t *)PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 128);
    if (p == NULL) {
        return 1;
    }
    for (int i = 0; i < 128; i++) {
        p[i] = (uint8_t)i;
    }
    for (int i = 0; i < 128; i++) {
        if (p[i] != (uint8_t)i) {
            PRT_MemFree(PRT_MEM_MID, p);
            return 1;
        }
    }
    if (PRT_MemFree(PRT_MEM_MID, p) != 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_002：size==0 应返回 NULL（与 FSC 行为对齐）。 */
int prt_mem_002(void)
{
    void *p = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 0);
    if (p != NULL) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    return 0;
}

/* prt_mem_003：分配后释放，再分配同样大小应成功（验证块复用/合并后可再分配）。 */
int prt_mem_003(void)
{
    void *p1 = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 256);
    if (p1 == NULL) {
        return 1;
    }
    if (PRT_MemFree(PRT_MEM_MID, p1) != 0) {
        return 1;
    }
    void *p2 = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 256);
    if (p2 == NULL) {
        return 1;
    }
    (void)memset(p2, 0xAA, 256);
    if (PRT_MemFree(PRT_MEM_MID, p2) != 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_004：对齐分配，校验返回地址满足对齐要求。 */
int prt_mem_004(void)
{
    uintptr_t align = (uintptr_t)16;
    uint8_t *p = (uint8_t *)PRT_MemAllocAlign(PRT_MEM_MID, PRT_MEM_PT, 64, MEM_ADDR_ALIGN_016);
    if (p == NULL) {
        return 1;
    }
    if (((uintptr_t)p & (align - 1)) != 0) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    (void)memset(p, 0x5A, 64);
    if (PRT_MemFree(PRT_MEM_MID, p) != 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_005：连续多次分配/释放，验证不会因碎片或统计错乱而失败。 */
int prt_mem_005(void)
{
    void *ptrs[8];
    int i;
    for (i = 0; i < 8; i++) {
        ptrs[i] = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 64 * (uint32_t)(i + 1));
        if (ptrs[i] == NULL) {
            printf("prt_mem_005 alloc failed i=%d size=%u\n", i, 64 * (uint32_t)(i + 1));
            for (int j = 0; j < i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
    }
    for (i = 0; i < 8; i++) {
        if (PRT_MemFree(PRT_MEM_MID, ptrs[i]) != 0) {
            printf("prt_mem_005 free failed i=%d size=%u ptr=%p\n", i, 64 * (uint32_t)(i + 1), ptrs[i]);
            return 1;
        }
    }
    /* 全部释放后再分配一个较大块，验证合并后大块可用。 */
    void *big = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 512);
    if (big == NULL) {
        printf("prt_mem_005 final alloc failed size=512\n");
        return 1;
    }
    if (PRT_MemFree(PRT_MEM_MID, big) != 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_006：realloc 扩容/缩容并校验内容保持（libc realloc，底层走 PRT_Mem）。 */
int prt_mem_006(void)
{
    unsigned char *p = (unsigned char *)realloc(NULL, 32);
    if (p == NULL) {
        return 1;
    }
    for (int i = 0; i < 32; i++) {
        p[i] = (unsigned char)(i + 1);
    }
    /* 扩容：原内容前 32 字节应保持 */
    p = (unsigned char *)realloc(p, 128);
    if (p == NULL) {
        return 1;
    }
    for (int i = 0; i < 32; i++) {
        if (p[i] != (unsigned char)(i + 1)) {
            free(p);
            return 1;
        }
    }
    /* 缩容：原内容前 16 字节应保持 */
    p = (unsigned char *)realloc(p, 16);
    if (p == NULL) {
        return 1;
    }
    for (int i = 0; i < 16; i++) {
        if (p[i] != (unsigned char)(i + 1)) {
            free(p);
            return 1;
        }
    }
    free(p);
    return 0;
}

/* prt_mem_007：超大 size 申请应返回 NULL（OOM 边界，两算法均不崩溃）。 */
int prt_mem_007(void)
{
    void *p = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 0x7FFFFFFFU);
    if (p != NULL) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    return 0;
}

/* prt_mem_008：多种对齐值分配，逐一校验返回地址对齐。 */
int prt_mem_008(void)
{
    static const enum MemAlign aligns[] = {
        MEM_ADDR_ALIGN_004, MEM_ADDR_ALIGN_008, MEM_ADDR_ALIGN_016,
        MEM_ADDR_ALIGN_032, MEM_ADDR_ALIGN_064,
    };
    static const uintptr_t masks[] = {0x3, 0x7, 0xF, 0x1F, 0x3F};
    void *ptrs[5];
    int i;

    for (i = 0; i < 5; i++) {
        ptrs[i] = PRT_MemAllocAlign(PRT_MEM_MID, PRT_MEM_PT, 64, aligns[i]);
        if (ptrs[i] == NULL) {
            for (int j = 0; j < i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
        if (((uintptr_t)ptrs[i] & masks[i]) != 0) {
            for (int j = 0; j <= i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
    }
    for (i = 0; i < 5; i++) {
        if (PRT_MemFree(PRT_MEM_MID, ptrs[i]) != 0) {
            return 1;
        }
    }
    return 0;
}

/* prt_mem_009：释放 NULL 应返回错误码（不崩溃）；与 FSC/TLSF 行为对齐。 */
int prt_mem_009(void)
{
    if (PRT_MemFree(PRT_MEM_MID, NULL) == 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_010：交错分配/释放压力，验证不损坏堆且最终全部可回收。 */
int prt_mem_010(void)
{
    void *ptrs[16];
    int i;

    for (i = 0; i < 16; i++) {
        ptrs[i] = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, (uint32_t)(32 + i * 16));
        if (ptrs[i] == NULL) {
            printf("prt_mem_010 alloc failed i=%d size=%u\n", i, (uint32_t)(32 + i * 16));
            for (int j = 0; j < i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
        (void)memset(ptrs[i], (i & 0xFF), (size_t)(32 + i * 16));
    }
    /* 隔一个释放一个，制造碎片 */
    for (i = 0; i < 16; i += 2) {
        if (PRT_MemFree(PRT_MEM_MID, ptrs[i]) != 0) {
            printf("prt_mem_010 first free failed i=%d ptr=%p\n", i, ptrs[i]);
            return 1;
        }
        ptrs[i] = NULL;
    }
    /* 再分配填补空洞 */
    for (i = 0; i < 16; i += 2) {
        ptrs[i] = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 48);
        if (ptrs[i] == NULL) {
            printf("prt_mem_010 refill alloc failed i=%d size=48\n", i);
            for (int j = 1; j < 16; j += 2) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
    }
    for (i = 0; i < 16; i++) {
        if (PRT_MemFree(PRT_MEM_MID, ptrs[i]) != 0) {
            printf("prt_mem_010 final free failed i=%d ptr=%p\n", i, ptrs[i]);
            return 1;
        }
    }
    return 0;
}

/* prt_mem_011：重复释放必须返回错误，不能把已释放块再次加入空闲链表。 */
int prt_mem_011(void)
{
    void *p = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 96);
    if (p == NULL) {
        return 1;
    }
    if (PRT_MemFree(PRT_MEM_MID, p) != 0) {
        return 1;
    }
    if (PRT_MemFree(PRT_MEM_MID, p) == 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_012：非法对齐枚举必须返回 NULL。 */
int prt_mem_012(void)
{
    void *p = PRT_MemAllocAlign(PRT_MEM_MID, PRT_MEM_PT, 64, MEM_ADDR_BUTT);
    if (p != NULL) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    return 0;
}

/* prt_mem_013：malloc_usable_size 普通分配至少覆盖申请大小，释放后不再使用。 */
int prt_mem_013(void)
{
    void *p = malloc(123);
    size_t usable;

    if (p == NULL) {
        return 1;
    }
    usable = malloc_usable_size(p);
    if (usable < 123) {
        free(p);
        return 1;
    }
    (void)memset(p, 0x3C, 123);
    free(p);
    return 0;
}

/* prt_mem_014：对齐分配也必须能被 malloc_usable_size 正确识别。 */
int prt_mem_014(void)
{
    void *p = PRT_MemAllocAlign(PRT_MEM_MID, PRT_MEM_PT, 100, MEM_ADDR_ALIGN_064);
    size_t usable;

    if (p == NULL) {
        return 1;
    }
    if (((uintptr_t)p & 0x3FU) != 0) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    usable = malloc_usable_size(p);
    if (usable < 100) {
        PRT_MemFree(PRT_MEM_MID, p);
        return 1;
    }
    (void)memset(p, 0xA5, 100);
    if (PRT_MemFree(PRT_MEM_MID, p) != 0) {
        return 1;
    }
    return 0;
}

/* prt_mem_015：小块批量分配释放，覆盖 slab 小块路径及普通算法的小块碎片路径。 */
int prt_mem_015(void)
{
    void *ptrs[64];
    uint32_t sizes[] = {8, 16, 24, 32, 48, 64, 96, 128};
    int i;

    for (i = 0; i < 64; i++) {
        uint32_t size = sizes[i % (sizeof(sizes) / sizeof(sizes[0]))];
        ptrs[i] = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, size);
        if (ptrs[i] == NULL) {
            printf("prt_mem_015 alloc failed i=%d size=%u\n", i, size);
            for (int j = 0; j < i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
        if (((uintptr_t)ptrs[i] & (sizeof(void *) - 1U)) != 0) {
            printf("prt_mem_015 align failed i=%d ptr=%p\n", i, ptrs[i]);
            for (int j = 0; j <= i; j++) {
                PRT_MemFree(PRT_MEM_MID, ptrs[j]);
            }
            return 1;
        }
        (void)memset(ptrs[i], i, size);
    }
    for (i = 63; i >= 0; i--) {
        if (PRT_MemFree(PRT_MEM_MID, ptrs[i]) != 0) {
            printf("prt_mem_015 free failed i=%d ptr=%p\n", i, ptrs[i]);
            return 1;
        }
    }
    return 0;
}

static void PrtMemTaskAllocEntry(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    TskHandle self;
    struct TskInfo info = {0};
    void *p;

    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;

    g_prtMemTaskResult = 1;
    if (PRT_TaskSelf(&self) != OS_OK) {
        g_prtMemTaskDone = 1;
        return;
    }
    if (PRT_TaskGetInfo(self, &info) != OS_OK || info.tcbAddr == NULL) {
        g_prtMemTaskDone = 1;
        return;
    }
    if (GET_HANDLE(self) >= OS_TSK_MAX_SUPPORT_NUM) {
        g_prtMemTaskDone = 1;
        return;
    }

    p = PRT_MemAlloc(PRT_MEM_MID, PRT_MEM_PT, 160);
    if (p == NULL) {
        g_prtMemTaskDone = 1;
        return;
    }
    (void)memset(p, 0x5C, 160);
    if (PRT_MemFree(PRT_MEM_MID, p) != 0) {
        g_prtMemTaskDone = 1;
        return;
    }

    g_prtMemTaskResult = 0;
    g_prtMemTaskDone = 1;
    (void)PRT_TaskDelay(OS_TICK_PER_SECOND);
}

/* prt_mem_016：在新建任务上下文中分配/释放，验证任务 API 适配不再是空桩。 */
int prt_mem_016(void)
{
    struct TskInitParam param = {0};
    TskHandle taskPid;
    U32 ret;

    g_prtMemTaskDone = 0;
    g_prtMemTaskResult = 1;

    param.taskEntry = PrtMemTaskAllocEntry;
    param.taskPrio = OS_TSK_PRIORITY_09;
    param.stackSize = 0x2000;
    param.name = "MemAllocTask";
    param.stackAddr = 0;

    ret = PRT_TaskCreate(&taskPid, &param);
    if (ret != OS_OK) {
        return 1;
    }

    ret = PRT_TaskResume(taskPid);
    if (ret != OS_OK) {
        (void)PRT_TaskDelete(taskPid);
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        if (g_prtMemTaskDone != 0) {
            break;
        }
        (void)PRT_TaskDelay(1);
    }

    ret = PRT_TaskDelete(taskPid);
    if (ret != OS_OK) {
        return 1;
    }

    return (g_prtMemTaskDone != 0 && g_prtMemTaskResult == 0) ? 0 : 1;
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int ret;
    test_run_main *run;

    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;

    printf("Start PRT_Mem testing....\n");

    for (int i = 0; i < sizeof(run_test_arry_prt) / sizeof(test_run_main *); i++) {
        run = run_test_arry_prt[i];
        printf("Runing %s test...\n", run_test_name_prt[i]);
        ret = run();
        if (ret != 0) {
            failCount++;
            printf("Run %s test fail\n", run_test_name_prt[i]);
        }
    }
    runCount += (int)(sizeof(run_test_arry_prt) / sizeof(test_run_main *));

    printf("Run total testcase %d, failed %d\n", runCount, failCount);
}
