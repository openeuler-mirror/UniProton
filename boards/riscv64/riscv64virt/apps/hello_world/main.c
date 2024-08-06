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
#include "os_cpu_riscv64.h"
#include "riscv.h"
#include "platform.h"
#include "uart.h"
#include "console.h"

// 初始配置函数入口
extern S32 OsConfigStart(void);


// 用户可以使用的接口，需要写入mtvec
extern void trap(); 

TskHandle g_pid;
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
    uart_printf("hard driver init end!!! test printf %d %p %x %f %lf\n",2,0x93,0x12,3.421,5.67);
    return OS_OK;
}



void thread_read_console(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    static char  buf[1030];
    uart_putstr_sync(">> ");
    while(1) {
        if(console_gets(buf,1030) != 0) {
            console_handle(buf);
        }
    }
}


U32 PRT_AppInit(void)
{
    console_init();
    struct TskInitParam para;
    para.taskEntry = thread_read_console;
    para.taskPrio = 25;
    para.stackSize = 0x1000;
    para.name ="console thread";
    para.stackAddr = 0;
    para.args[0] = 0;
    para.args[1] = 0;
    para.args[2] = 0;
    para.args[3] = 0;
    
    if(PRT_TaskCreate(&g_pid,&para) != OS_OK) {
        uart_putstr_sync("err in prt_task_create");
        while(1) {
            OS_EMBED_ASM("wfi");
        }
    }
    if(PRT_TaskResume(g_pid) != OS_OK ) {
        uart_putstr_sync("err in prt_task_resume");
        while(1) {
            OS_EMBED_ASM("wfi");
        }
    }
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
