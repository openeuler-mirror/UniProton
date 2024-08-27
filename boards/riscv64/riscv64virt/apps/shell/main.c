/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-03-07
 * Description: 初始化内核，同时初始化shell,然后执行第一个线程
 */

#include "prt_task.h"
#include "prt_config.h"
#include "prt_typedef.h"
#include "os_cpu_riscv64.h"
#include "riscv.h"
#include "platform.h"
#include "uart.h"
#include "console.h"
#include "stdio.h"
#include "shell.h"
#include "show.h"
extern int OsShellCmdMemInfo(int argc, const char **argv);
extern U32 OsShellCmdHelp(U32 argc, const char **argv);
extern int OsShellCmdTaskInfo(int argc, const char **argv);
// 初始配置函数入口
extern S32 OsConfigStart(void);


// 用户可以使用的接口，需要写入mtvec
extern void trap(); 

void OsHwInit()
{
    w_mstatus(r_mstatus() & (~MIE));
    w_mie(r_mie() | MEIE | MSIE | MTIE);
    U64 x = (U64)trap;
    OS_EMBED_ASM("csrw mtvec, %0"::"r"(x):);
    //w_mtvec((U64)trap);
    U64 coreId = PRT_GetCoreID();
    U64 nowTime = *(U64 *)CLINT_TIME;
    *(U64* )CLINT_TIMECMP(coreId) = (nowTime + OS_TICK_PER_SECOND);    
}

U32 PRT_HardDrvInit(void)
{
    uart_init(NULL);
    printf("shell test! %p %p %p",OsShellCmdMemInfo,OsShellCmdHelp,OsShellCmdTaskInfo);
    return OS_OK;
}


U32 PRT_AppInit(void)
{
    console_init();
    int ret = OsShellInit(0);
    if(ret !=0)
    {
	uart_printf("shell init error !\n");
    	while(1)
	{
		__asm__ __volatile__("wfi");
	}
    }

    return OS_OK;
}

int main()
{
    double x = 1.3;
    double y = 2.4;
    float z = x + y; 
	 z -=1;
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
    if( dest == NULL || src == NULL) {
    	return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
