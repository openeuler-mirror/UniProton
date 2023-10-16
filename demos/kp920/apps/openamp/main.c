#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "test.h"
#include "mmu.h"
#include "cpu_config.h"

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

extern U32 PRT_PrintfInit();

static void test_pcie_mmu(void)
{
    S32 ret;
    U32 i;
    unsigned long* ptr = MMU_TEST_ADDR;

    ret = mmu_request_no_lock(ptr, 0x10000);
    if (ret != 0) {
        printf("mmu_request_no_lock error!! ret = %d\r\n", ret);
        return;
    }
    ret = mmu_update();
    if (ret != 0) {
        printf("mmu_update error!! ret = %d\r\n", ret);
        return;
    }

    for (i = 0; i < 100; i++) {
        ptr[i] = 0xaa550000aa550000 + i;
    }
}

#define PCI_ECAM_ADDRESS(Bus, Device, Function, Offset) \
  (((Offset) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

static void test_pcie_ecam(void)
{
    U32 *ptr;
    U32 i;

    ptr = (U32*)MMU_ECAM_ADDR;
    printf("\r\necam_dump_200:\r\n");
    for (i = 0; i < 0x200; i++) {
        printf("0x%08x ", ptr[i], (i % 0x10 == 0) ? "\r\n" : "");
    }
}

#if defined(OS_OPTION_PCIE)
extern void test_pcie(void);
#endif
// extern int pci_msg_send(void *data, size_t len);

// struct pci_rpmsg_s {
//     int cmd;
//     char data[100];
// };

// void test_pcie_msg(void)
// {
//     struct pci_rpmsg_s rpmsg = {
//         .cmd = 10,
//         .data = {
//             [0 ... 99] = 0xa5,
//         },
//     };
//     printf("client send msg!\r\n");
//     pci_msg_send(&rpmsg, sizeof(rpmsg));
// }

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
    for (int i = 1; i < 5; i++) {
        printf("TestTaskEntry=============TestTaskEntry\r\n");
    }
#if defined(OS_OPTION_PCIE)
    test_pcie();
#endif
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

