#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "test.h"
#include "cpu_config.h"

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

extern U32 PRT_PrintfInit();

#if defined(OS_OPTION_PCIE)
extern void test_pcie(void);
#endif

#if 0 // defined(OS_OPTION_PCIE)
void test_gic_its(void)
{
    uint32_t value, n;

    printf("GITS_IIDR:0x%08x\n", GIC_REG_READ(GITS_IIDR));
    printf("GITS_TYPER:0x%08x\n", GIC_REG_READ(GITS_TYPER));
    for (n = 0; n < 8; n++) {
        printf("GITS_PIDR(%u):0x%08x\n", n, GIC_REG_READ(GITS_PIDR(n)));
    }

    printf("GITS1_IIDR:0x%08x\n", GIC_REG_READ(GITS1_IIDR));
    printf("GITS1_TYPER:0x%08x\n", GIC_REG_READ(GITS1_TYPER));
    for (n = 0; n < 8; n++) {
        printf("GITS1_PIDR(%u):0x%08x\n", n, GIC_REG_READ(GITS1_PIDR(n)));
    }
}
#endif

#if defined(OS_OPTION_OPENAMP) || defined(OS_OPTION_OPENAMP_PROXYBASH)
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

#if defined(OS_OPTION_OPENAMP_PROXYBASH)
extern int proxybash_exec(char *cmdline, char *result_buf, unsigned int buf_len);
#endif

extern U64 g_origin_propbase;
extern U64 g_origin_pendbase;

extern int irq_range_idx;
extern unsigned int irq_range_log[];
void TestTaskEntry()
{
#if defined(OS_OPTION_OPENAMP) || defined(OS_OPTION_OPENAMP_PROXYBASH)
    TestOpenamp();
#endif
    int tick_cnt = 1;
    for (int i = 0; i < 3; i++) {
        printf("TestTaskEntry=============TestTaskEntry\r\n");
        PRT_TaskDelay(tick_cnt);
        tick_cnt = tick_cnt * 10;
    }
#if defined(OS_OPTION_PCIE)
    //test_gic_its();
    test_pcie();
#endif

#if 0 // defined(OS_OPTION_OPENAMP_PROXYBASH)
    char *cmdline = "cat /proc/iomem | grep 'PCI ECAM'";
    char result_buf[0x800];
    unsigned int buf_len = sizeof(result_buf);
    int ret = proxybash_exec(cmdline, result_buf, buf_len);
    if (ret < 0) {
        printf("proxybash_exec fail, ret:0x%x", ret);
    } else {
        printf("proxybash_exec result(%u): %02x %02x %02x %02x %02x %02x %02x %02x",
            ret, result_buf[0], result_buf[1], result_buf[2], result_buf[3],
            result_buf[4], result_buf[5], result_buf[6], result_buf[7]);
    }
#endif

    printf("0x%llx,0x%llx,\r\n", g_origin_propbase, g_origin_pendbase);
    do {
        if (irq_range_idx > 0) {
            for (irq_range_idx; irq_range_idx > 0; irq_range_idx--) {
                printf("%u,", irq_range_log[irq_range_idx - 1]);
            }
        }
        PRT_TaskDelay(100);
    } while(1);

}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
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
    return ret;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
}

