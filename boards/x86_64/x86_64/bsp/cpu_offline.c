#include <stdio.h>
#include "prt_hwi.h"
#include "prt_task.h"

#define CPU_OFFLINE_HWI 0x2

void NmiIsr(void)
{
    printf("Rtos Stop ... \r\n");
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

void CpuOfflineHwiCreate(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(CPU_OFFLINE_HWI, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("line %d, errno: 0x%08x\n", __LINE__, ret);
        return;
    }

    ret = PRT_HwiCreate(CPU_OFFLINE_HWI, NmiIsr, 0);
    if (ret != OS_OK) {
        printf("line %d, errno: 0x%08x, create hwi failed\n", __LINE__, ret);
        return;
    }
}
