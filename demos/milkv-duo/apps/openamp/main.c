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
 * Create: 2024-02-22
 * Description: main函数入口,初始化内核以及执行console_init()函数
 */
#include "prt_task.h"
#include "prt_config.h"
#include "prt_typedef.h"
#include "prt_mem.h"

#include "csr.h"
#include "irq.h"
#include "uart.h"
#include "rtos_cmdqu.h"
#include "pedestal_function.h"
#include "hal_uart_dw.h"
// 初始配置函数入口
extern S32 OsConfigStart(void);
extern void trap(void); 
extern U32 RpmsgHwiInit(void);
extern U32 rpmsg_service_init(void);

struct pedestal_operation* g_ped_ops;

static TskHandle g_testTskHandle;

static char data_test[10] = {0x34, 0x58, 0xff, 0xaf, 0xae, 0x1a, 0x6f, 0xef, 0x18, 0x43};

void TestTaskEntry()
{
    int ret;

    ret = rpmsg_service_init();
    if (ret) {
        uart_printf("rpmsg_service init failed!\n");
	return;
    }
    //uart_printf("rpmsg_service init success!\n");
}
static U8 bss_test[12];
extern char __bss_start__[];
extern char __bss_end__[];
void OsHwInit()
{
    U64 pc;
    __asm__ __volatile__("auipc %0, 0":"=r"(pc)::);
    uart_init();
    //uart_printf("printf test !! %d %lf %x\n", 30, 0.34, 0x1234);
    //uart_printf("pc %p\n", pc);
    char pass_bss_test = 1;
    char pass_data_test = 1;
    for(int i=0;i<12;i++)
    {
	if(bss_test[i] != 0)
	{
		pass_bss_test = 0;
		break;
	}
    }
    if(!(data_test[0] == 0x34 && data_test[1] == 0x58 && data_test[2] == 0xff && data_test[3] == 0xaf && data_test[4] == 0xae && data_test[5] == 0x1a && data_test[6] == 0x6f && data_test[7] == 0xef && data_test[8] == 0x18 && data_test[9] == 0x43) )
    {
        pass_data_test =0;
    }
    if(pass_bss_test)
        uart_printf("pass bss init test!\n");
    else
        uart_printf("bss init test failed!\n");
    if(pass_data_test)
        uart_printf("pass data init test!\n");
    else
        uart_printf("data init test failed!\n");

    if(!(pass_bss_test && pass_data_test))
	while(1) __asm__ __volatile__("wfi");
    write_csr(mtvec, trap);
    U64 time_now;
    CLINT_MTIME(time_now);
    time_now += OS_SYS_CLOCK/OS_TICK_PER_SECOND;
    *((volatile U32*)CLINT_TIMECMPL0) = (U32)(time_now & 0xffffffff);
    *((volatile U32*)CLINT_TIMECMPH0) = (U32)((time_now >> 32) & 0xffffffff);
    //uart_printf("OsHwInit done!\n");
    return;
}


U32 PRT_HardDrvInit(void)
{
    rtos_cmdqu_init();
    //uart_printf("PRT_HardDrvInit done!\n");
    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    ret = OsShellInit(0);
    if(ret != OS_OK)
    {
	uart_printf("Shell Init Error!\n");
	return ret;
    }
    ret = RpmsgHwiInit();
    if(ret != OS_OK)
    {
        uart_printf("RpmsgHwiInit failed!\n");
	return ret;
    }
    //uart_printf("RpmsgHwiInit Success!\n");   
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 25;
    param.name = "mica_testTask";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret) 
    {
        uart_printf("create task failed!\n");
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle);
    if (ret) 
    {
        uart_printf("resume task failed!\n");
        return ret;
    }

    //uart_printf("PRT_AppInit done!\n");
    return OS_OK;
}

int main()
{
    return OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL) {
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
    if(dest == NULL || src == NULL) {
    	return NULL;
    } 
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
void os_asm_invalidate_dcache_all()
{
	return;
}
