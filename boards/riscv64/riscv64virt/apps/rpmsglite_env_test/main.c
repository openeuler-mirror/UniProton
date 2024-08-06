/*
 * Copyright (c) 2009-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-4-26
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

#include "rpmsg_common_extern.h"

// 初始配置函数入口
extern S32 OsConfigStart(void);


// 用户可以使用的接口，需要写入mtvec
extern void trap(); 

TskHandle master_pid;
TskHandle remote_pid;
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

U32 PRT_AppInit(void)
{
    struct TskInitParam para;
    para.stackSize = 0x1000 << 8;
    para.name ="rpmsg thread";
    para.stackAddr = 0;
    para.args[0] = 0;
    para.args[1] = 0;
    para.args[2] = 0;
    para.args[3] = 0;

#if defined(CONFIG_RPMSGLITE_MASTERPRJ)
   para.taskPrio = 25;
   para.taskEntry = rpmsg_master_rpc_on_que;
   if(PRT_TaskCreate(&master_pid,&para) != OS_OK) {
       uart_putstr_sync("err in rpmsg_master create");
       goto failed;
   }
   if(PRT_TaskResume(master_pid) != OS_OK) {
        uart_putstr_sync("err in rpmsg_master/remote resume");
        goto failed;
    }
#elif defined(CONFIG_RPMSGLITE_SLAVEPRJ)
    para.taskPrio = 25;
    para.taskEntry = rpmsg_remote_rpc_on_que;
    if(PRT_TaskCreate(&remote_pid,&para) != OS_OK) {
        uart_putstr_sync("err in rpmsg_remote create");
        goto failed;
    }
    if(PRT_TaskResume(remote_pid) != OS_OK) {
        uart_putstr_sync("err in rpmsg_master/remote resume");
        goto failed;
    }
#else 
    para.taskPrio = 24;
    para.taskEntry = rpmsg_master;
    if(PRT_TaskCreate(&master_pid,&para) != OS_OK) {
        uart_putstr_sync("err in rpmsg_master create");
        goto failed;
    }
    para.taskPrio = 25;
    para.taskEntry = rpmsg_remote;
    if(PRT_TaskCreate(&remote_pid,&para) != OS_OK) {
        uart_putstr_sync("err in rpmsg_remote create");
        goto failed;
    }

    if(PRT_TaskResume(master_pid) != OS_OK
       || PRT_TaskResume(remote_pid) != OS_OK) {
        uart_putstr_sync("err in rpmsg_master/remote resume");
        goto failed;
    }
#endif
    return OS_OK;
failed:
    while (1) {
        OS_EMBED_ASM("wfi");
    }
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
