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
 * Description: trap相关的函数
 */
#include "os_exc_riscv64.h"
#include "os_cpu_riscv64.h"
#include "prt_typedef.h"
#include "prt_config.h"
#include "prt_tick.h"
#include "prt_buildef.h"
#include "uart.h"
#include "irq.h"
#include "csr.h"

#define OS_IS_INTERUPT(mcause)     (mcause & 0x8000000000000000ull)
#define OS_IS_EXCEPTION(mcause)    (~(OS_IS_INTERUPT))
#define OS_IS_TICK_INT(mcause)     (mcause == 0x8000000000000007ull)
#define OS_IS_SOFT_INT(mcause)     (mcause == 0x8000000000000003ull)
#define OS_IS_EXT_INT(mcause)      (mcause == 0x800000000000000bull)
#define OS_IS_TRAP_USER(mcause)    (mcause == 0x000000000000000bull)

extern void OsExcDispatch();       
extern void hwi_handler();   
extern void hwi_timer_handler();  


void HwTimerIsr(void)
{ 
    U64 time_now;
    CLINT_MTIME(time_now);
    time_now += OS_SYS_CLOCK/OS_TICK_PER_SECOND;
    *((volatile U32*)CLINT_TIMECMPL0) = (U32)(time_now & 0xffffffff);
    *((volatile U32*)CLINT_TIMECMPH0) = (U32)((time_now >> 32) & 0xffffffff);
    PRT_TickISR();
}

void trap_entry(U64 mcause)
{
    if(OS_IS_INTERUPT(mcause)) 
    {
        if(OS_IS_TICK_INT(mcause)) 
        {
            hwi_timer_handler();
        } 
        else if(OS_IS_SOFT_INT(mcause)) 
        {
            OsExcDispatch();
        } 
        else if(OS_IS_EXT_INT(mcause)) 
        {
	    //uart_printf("external interrupt occur!\n");
            hwi_handler();
        }
    } 
    else 
    {
	U64 mtval = read_csr(mtval);
	U64 mepc = read_csr(mepc);
	U64 mcause = read_csr(mcause);
	uart_printf("in exception!!!!!!!!!!!! mcause: %p mtval %p mepc %p\n", mcause, mtval ,mepc);
        OsExcDispatch();
    }
}
