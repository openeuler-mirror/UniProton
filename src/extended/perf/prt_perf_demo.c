/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-09
 * Description: Perf示例代码
 */

#include <fcntl.h>
#include "prt_perf.h"
#include "prt_task.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "securec.h"
#include "prt_timer.h"
#include "prt_attr_external.h"
#include "prt_plist_external.h"
#include "include/prt_perf_comm.h"

#define PERF_PC_TOP_NUM 10
#define PERF_PC_MAX_NUM 1000
#define PERF_BUFF_LEN 0x10000
#define PERF_TASK_STACKSIZE 0x30000

struct TagSamplePcCb {
    struct TagListObject pcList;
    uintptr_t pc;
    U32 count;
};

// 定义非static的全局变量做测试，用局部变量会被优先
OS_SEC_BSS U32 g_perfTestArry[100];
OS_SEC_BSS U32 g_pcNum;
OS_SEC_BSS static struct TagSamplePcCb g_pcTotal[PERF_PC_MAX_NUM];
OS_SEC_BSS static struct TagSamplePcCb g_pcTop[PERF_PC_TOP_NUM];
OS_SEC_BSS static struct TagListObject g_perfList;
OS_SEC_BSS bool g_perfTaskEndFlag[3];

static void OsOutPutPcList()
{
    U32 i;
    U32 max;
    struct TagSamplePcCb *sampleCb = NULL;
    struct TagSamplePcCb *foundSampleCb = NULL;

    for (i = 0; i < PERF_PC_TOP_NUM && !ListEmpty(&g_perfList); i++) {
        max = 0;
        foundSampleCb = NULL;
        LIST_FOR_EACH(sampleCb, &g_perfList, struct TagSamplePcCb, pcList) {
            if (sampleCb->count > max) {
                max = sampleCb->count;
                foundSampleCb = sampleCb;
            }
        }
        if (foundSampleCb != NULL) {
            g_pcTop[i].pc = foundSampleCb->pc;
            g_pcTop[i].count = foundSampleCb->count;
            ListDelete(&foundSampleCb->pcList);
        }
    }

    for (i = 0; i < PERF_PC_TOP_NUM; i++) {
        PRT_Printf("perf top pc=0x%llx, count=%u\n", g_pcTop[i].pc, g_pcTop[i].count);
    }

    return;
}

static void OsAddPcToList(const char *pc)
{
    bool found = FALSE;
    uintptr_t value = 0;
    struct TagSamplePcCb *tmpSampleCb = NULL;
    struct TagSamplePcCb *newSampleCb = NULL;

    memcpy_s(&value, sizeof(uintptr_t), pc, sizeof(uintptr_t));

    LIST_FOR_EACH(tmpSampleCb, &g_perfList, struct TagSamplePcCb, pcList) {
        if (tmpSampleCb->pc == value) {
            found = TRUE;
            break;
        }
        if (tmpSampleCb->pc > value) {
            break;
        }
    }

    if (found) {
        tmpSampleCb->count++;
    } else {
        if (g_pcNum >= PERF_PC_MAX_NUM - 1) {
            return;
        }

        newSampleCb = ((struct TagSamplePcCb *)g_pcTotal) + g_pcNum;
        newSampleCb->pc = value;
        newSampleCb->count = 1;
        if (ListEmpty(&g_perfList)) {
            ListTailAdd(&newSampleCb->pcList, &g_perfList);
        } else {
            ListTailAdd(&newSampleCb->pcList, &tmpSampleCb->pcList);
        }
        g_pcNum++;
    }

    return;
}

static void OsOutputCallChain(const char *callChain, FILE *fp)
{
    U32 i;
    uintptr_t depth = 0;
    char *curPoint = callChain;

    memcpy_s(&depth, sizeof(uintptr_t), curPoint, sizeof(uintptr_t));
    curPoint += sizeof(uintptr_t);
    static U32 x = 0;
    x++;
    PRT_Printf("UniProton    888  %u:     250000 task-clock:ppp:\n", x);
    if (fp != NULL) {
        fprintf(fp, "UniProton    888  %u:     250000 task-clock:ppp:\n", x);
    }

    for (i = 0; i < depth; i++) {
        PRT_Printf("1 0x%llx (1)\n", *(uintptr_t*)curPoint);
        if (fp != NULL) {
            fprintf(fp, "1 0x%llx (1)\n", *(uintptr_t*)curPoint);
        }
        curPoint += sizeof(uintptr_t);
    }

    PRT_Printf("\n");
    if (fp != NULL) {
        fprintf(fp, "\n");
    }

    return;
}

static void OsBufferParser(const char *buf, U32 len)
{
    char *curPoint = buf;
    struct TagSamplePcCb *pcNode = NULL;
    U32 headLen = sizeof(PerfDataHdr);
    U32 sampleDataLen = sizeof(PerfSampleData);
    FILE *fp = fopen("/tmp/output.perf", "w");
    if (fp == NULL) {
        PRT_Printf("fopen output.perf failed\n");
    }

    curPoint += headLen;
    len -= headLen;

    while (len >= sampleDataLen) {
        curPoint += sampleDataLen;
        len -= sampleDataLen;
        OsAddPcToList(curPoint - sizeof(PerfBackTrace) - sizeof(uintptr_t));
        OsOutputCallChain(curPoint - sizeof(PerfBackTrace), fp);
    }

    if (fp != NULL) {
        fclose(fp);
    }

    return;
}

void OsPrintBuff(const char *buf, U32 len)
{
    U32 i = 0;

    if (len < sizeof(PerfDataHdr)) {
        PRT_Printf("perf data length less than header size\n");
        return;
    }

    OS_LIST_INIT(&g_perfList);
    OsBufferParser(buf, len);

    PRT_Printf("len=%u\n", len);

    OsOutPutPcList();

    return;
}

__attribute__((noinline)) void foo1()
{
    for (U32 i = 0; i < 10000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) void bar()
{
    for (U32 i = 0; i < 20000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) void boo()
{
    for (U32 i = 0; i < 10000; i++) {
        foo1();
    }
    for (U32 i = 0; i < 10000; i++) {
        bar();
    }
}

__attribute__((noinline)) void OsPerfTest()
{
    boo();
}

__attribute__((noinline)) void TaskEntryA_7()
{
    for (U32 i = 0; i < 10000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) void TaskEntryA_8()
{
    for (U32 i = 0; i < 20000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) U32 TaskEntryA_6()
{
    for (U32 i = 0; i < 10000; i++) {
        TaskEntryA_7();
        TaskEntryA_8();
        PRT_TaskDelay(0);
    }

    return 0;
}

__attribute__((noinline)) U32 TaskEntryA_5()
{
    U32 ret;
    ret = TaskEntryA_6();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryA_4()
{
    U32 ret;
    ret = TaskEntryA_5();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryA_3()
{
    U32 ret;
    ret = TaskEntryA_4();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryA_2()
{
    U32 ret;
    ret = TaskEntryA_3();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryA_1()
{
    U32 ret;
    ret = TaskEntryA_2();
    ret++;
    return ret;
}

__attribute__((noinline)) void DemoTaskEntryA()
{
    PRT_Printf("DemoTaskEntryA Start\n");
    U32 ret;
    ret = TaskEntryA_1();
    g_perfTaskEndFlag[0] = true;
    PRT_Printf("DemoTaskEntryA End\n");
}

__attribute__((noinline)) void OsPerfTestA()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam taskParam = {0};
    TskHandle taskId;

    taskParam.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x3000, MEM_ADDR_ALIGN_016);
    taskParam.taskEntry = (TskEntryFunc)DemoTaskEntryA;
    taskParam.taskPrio = 12;
    taskParam.name = "PerfDemoTaskA";
    taskParam.stackSize = 0x3000;

    ret = PRT_TaskCreate(&taskId, &taskParam);
    if (ret) {
        PRT_Printf("perf demo task A create failed, ret = %d\n", ret);
    }

    ret = PRT_TaskResume(taskId);
    if (ret) {
        PRT_Printf("perf demo task A resume failed, ret = %d\n", ret);
    }
}

__attribute__((noinline)) void TaskEntryB_7()
{
    for (U32 i = 0; i < 10000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) void TaskEntryB_8()
{
    for (U32 i = 0; i < 20000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) U32 TaskEntryB_6()
{
    for (U32 i = 0; i < 10000; i++) {
        TaskEntryB_7();
        TaskEntryB_8();
        PRT_TaskDelay(0);
    }

    return 0;
}

__attribute__((noinline)) U32 TaskEntryB_5()
{
    U32 ret;
    ret = TaskEntryB_6();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryB_4()
{
    U32 ret;
    ret = TaskEntryB_5();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryB_3()
{
    U32 ret;
    ret = TaskEntryB_4();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryB_2()
{
    U32 ret;
    ret = TaskEntryB_3();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryB_1()
{
    U32 ret;
    ret = TaskEntryB_2();
    ret++;
    return ret;
}

__attribute__((noinline)) void DemoTaskEntryB()
{
    PRT_Printf("DemoTaskEntryB Start\n");
    U32 ret;
    ret = TaskEntryB_1();
    g_perfTaskEndFlag[1] = true;
    PRT_Printf("DemoTaskEntryB End\n");
}

__attribute__((noinline)) void OsPerfTestB()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam taskParam = {0};
    TskHandle taskId;

    taskParam.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x3000, MEM_ADDR_ALIGN_016);
    taskParam.taskEntry = (TskEntryFunc)DemoTaskEntryB;
    taskParam.taskPrio = 12;
    taskParam.name = "PerfDemoTaskB";
    taskParam.stackSize = 0x3000;

    ret = PRT_TaskCreate(&taskId, &taskParam);
    if (ret) {
        PRT_Printf("perf demo task B create failed, ret = %d\n", ret);
    }

    ret = PRT_TaskResume(taskId);
    if (ret) {
        PRT_Printf("perf demo task B resume failed, ret = %d\n", ret);
    }
}

__attribute__((noinline)) void TaskEntryC_7()
{
    for (U32 i = 0; i < 10000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) void TaskEntryC_8()
{
    for (U32 i = 0; i < 20000; i++) {
        g_perfTestArry[i % 100] = i * i;
    }
}

__attribute__((noinline)) U32 TaskEntryC_6()
{
    for (U32 i = 0; i < 10000; i++) {
        TaskEntryC_7();
        TaskEntryC_8();
        PRT_TaskDelay(0);
    }

    return 0;
}

__attribute__((noinline)) U32 TaskEntryC_5()
{
    U32 ret;
    ret = TaskEntryC_6();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryC_4()
{
    U32 ret;
    ret = TaskEntryC_5();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryC_3()
{
    U32 ret;
    ret = TaskEntryC_4();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryC_2()
{
    U32 ret;
    ret = TaskEntryC_3();
    ret++;
    return ret;
}

__attribute__((noinline)) U32 TaskEntryC_1()
{
    U32 ret;
    ret = TaskEntryC_2();
    ret++;
    return ret;
}

__attribute__((noinline)) void DemoTaskEntryC()
{
    PRT_Printf("DemoTaskEntryC Start\n");
    U32 ret;
    ret = TaskEntryC_1();
    g_perfTaskEndFlag[2] = true;
    PRT_Printf("DemoTaskEntryC End\n");
}

__attribute__((noinline)) void OsPerfTestC()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam taskParam = {0};
    TskHandle taskId;

    taskParam.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x3000, MEM_ADDR_ALIGN_016);
    taskParam.taskEntry = (TskEntryFunc)DemoTaskEntryC;
    taskParam.taskPrio = 12;
    taskParam.name = "PerfDemoTaskC";
    taskParam.stackSize = 0x3000;

    ret = PRT_TaskCreate(&taskId, &taskParam);
    if (ret) {
        PRT_Printf("perf demo task C create failed, ret = %d\n", ret);
    }

    ret = PRT_TaskResume(taskId);
    if (ret) {
        PRT_Printf("perf demo task C resume failed, ret = %d\n", ret);
    }
}

static void perfTestTimedEvent()
{
    U32 ret;
    U32 i;
    U32 j;
    U32 len;
    char buf[0x10000] = {0};

    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_TIMED,
            .events = {
                [0] = {PERF_COUNT_CPU_CLOCK, 100},
            },
            .eventsNr = 1,
            .predivided = 0,            /* cycle counter increase every 64 cycles */
        },
        .taskIds = {0},
        .taskIdsNr = 0,

        .needStoreToBuffer = TRUE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        PRT_Printf("demo perf config timer event error 0x%x\n", ret);
        return;
    }

    PRT_PerfStart(0);
    OsPerfTestA();
    OsPerfTestB();
    OsPerfTestC();
    while (!g_perfTaskEndFlag[0] || !g_perfTaskEndFlag[1] || !g_perfTaskEndFlag[2]) {
        PRT_TaskDelay(0);
    }
    PRT_PerfStop();

    len = PRT_PerfDataRead(buf, 0x10000);
    OsPrintBuff(buf, len);

    return;
}

static void perfTestSwEvent()
{
    U32 ret;
    void *buf = NULL;

    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_SW,
            .events = {
                [0] = {PERF_COUNT_SW_TASK_SWITCH, 1},
                [1] = {PERF_COUNT_SW_HWI_RESPONSE_IN, 10000},
                [2] = {PERF_COUNT_SW_MEM_ALLOC, 1},
                [3] = {PERF_COUNT_SW_SEM_PEND, 1},
            },
            .eventsNr = 4,
            .predivided = 0,            /* cycle counter increase every 64 cycles */
        },
        .taskIds = {0},
        .taskIdsNr = 0,
        .needStoreToBuffer = FALSE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        PRT_Printf("demo perf config sw event error 0x%x\n", ret);
        return;
    }

    // task switch
    PRT_PerfStart(1);
    PRT_TaskDelay(5);
    PRT_TaskDelay(5);
    PRT_TaskDelay(5);

    // malloc
    buf = PRT_MemAlloc(OS_MID_PERF, OS_MEM_DEFAULT_PT0, 10);
    if (buf == NULL) {
        printf("perf sw event demo malloc failed\n");
        goto PERF_SW_STOP;
    }
    (void)PRT_MemFree(OS_MID_PERF, buf);

    // sem
    static SemHandle sem;
    ret = PRT_SemCreate(1, &sem);
    if (ret != OS_OK) {
        PRT_Printf("perf sw event demo create sem failed %d, ret = 0x%x\n", ret);
        goto PERF_SW_STOP;
    }
    ret = PRT_SemPend(sem, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        PRT_Printf("perf sw event demo pend sem failed %d, ret = 0x%x\n", ret);
        goto PERF_DEL_SEM;
    }

    ret = PRT_SemPost(sem);
    if (ret != OS_OK) {
        PRT_Printf("perf sw event demo post sem failed %d, ret = 0x%x\n", ret);
        goto PERF_DEL_SEM;
    }

    // irq
    PRT_TaskDelay(1000);

PERF_DEL_SEM:
    ret = PRT_SemDelete(sem);
    if (ret != OS_OK) {
        PRT_Printf("perf sw event demo del sem failed %d, ret = 0x%x\n", ret);
    }
PERF_SW_STOP:
    PRT_Printf("perf sw event stopping...\n");

    PRT_PerfStop();

    return;
}

static void perfTestHwEvent()
{
    U32 ret;
    U32 len;
    char buf[0x10000] = {0};

    PerfConfigAttr attr = {
        .eventsCfg = {
            .type = PERF_EVENT_TYPE_HW,
            .events = {
                [0] = {PERF_COUNT_HW_CPU_CYCLES, 0xFFFF},
                [1] = {PERF_COUNT_HW_INSTRUCTIONS, 0xFFFFFF00},
                [2] = {PERF_COUNT_HW_DCACHE_REFERENCES, 0xFFFFFF00},
                [3] = {PERF_COUNT_HW_DCACHE_MISSES, 0xFFFFFF00},
                [4] = {PERF_COUNT_HW_ICACHE_REFERENCES, 0xFFFFFF00},
                [5] = {PERF_COUNT_HW_ICACHE_MISSES, 0xFFFFFF00},
                [6] = {PERF_COUNT_HW_BRANCH_INSTRUCTIONS, 0xFFFFFF00},
            },
            .eventsNr = 7,
            .predivided = 1,            /* cycle counter increase every 64 cycles */
        },
        .taskIds = {0},
        .taskIdsNr = 0,
        .needStoreToBuffer = FALSE,
        .sampleType = 0xFFFFFFFF,
    };

    ret = PRT_PerfConfig(&attr);
    if (ret != OS_OK) {
        PRT_Printf("demo perf config error 0x%u\n", ret);
        return;
    }

    PRT_Printf("------------direct print mode------------\n");
    PRT_PerfStart(1);
    OsPerfTest();
    PRT_Printf("perf hw event stopping...\n");
    PRT_PerfStop();

    PRT_Printf("------------store to buffer mode------------\n");
    attr.needStoreToBuffer = TRUE;
    PRT_PerfConfig(&attr);
    PRT_PerfStart(1);
    OsPerfTest();
    PRT_PerfStop();

    len = PRT_PerfDataRead(buf, 0x10000);
    OsPrintBuff(buf, len);

    return;
}

void DemoTaskEntry()
{
    U32 ret;
    char buffer[PERF_BUFF_LEN];

    memset_s(buffer, PERF_BUFF_LEN, 0, PERF_BUFF_LEN);
    ret = PRT_PerfInit(buffer, PERF_BUFF_LEN);
    if (ret != OS_OK) {
        PRT_Printf("Perf init failed, ret = 0x%x\n", ret);
    }

#ifdef OS_OPTION_PERF_HW_PMU
    perfTestHwEvent();
#elif defined(OS_OPTION_PERF_SW_PMU)
    perfTestSwEvent();
#elif defined(OS_OPTION_PERF_TIMED_PMU)
    perfTestTimedEvent();
#else
    PRT_Printf("Perf type doesn't configured\n");
#endif

    return;
}

void PerfDemoTask()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam taskParam = {0};
    TskHandle taskId;

    taskParam.stackAddr = PRT_MemAllocAlign(0, ptNo, PERF_TASK_STACKSIZE, MEM_ADDR_ALIGN_016);
    taskParam.taskEntry = (TskEntryFunc)DemoTaskEntry;
    taskParam.taskPrio = 12;
    taskParam.name = "PerfDemoTask";
    taskParam.stackSize = PERF_TASK_STACKSIZE;

#if (PRTCFG_KERNEL_SMP == YES)
    taskParam.usCpuAffiMask = CPUID_TO_AFFI_MASK(0);
#endif

    ret = PRT_TaskCreate(&taskId, &taskParam);
    if (ret) {
        PRT_Printf("perf test task create failed\n");
        return;
    }

    PRT_Printf("Perf test task id = %u\n", taskId);
    ret = PRT_TaskResume(taskId);
    if (ret) {
        PRT_Printf("perf test task resume failed\n");
        return;
    }
}