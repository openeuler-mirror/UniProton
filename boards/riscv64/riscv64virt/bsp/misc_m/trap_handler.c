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
#include "platform.h"
#include "prt_config.h"
#include "prt_tick.h"

#define OS_IS_INTERUPT(mcause)     (mcause & 0x8000000000000000ull)
#define OS_IS_EXCEPTION(mcause)    (~(OS_IS_INTERUPT))
#define OS_IS_TICK_INT(mcause)     (mcause == 0x8000000000000007ull)
#define OS_IS_SOFT_INT(mcause)     (mcause == 0x8000000000000003ull)
#define OS_IS_EXT_INT(mcause)      (mcause == 0x800000000000000bull)
#define OS_IS_TRAP_USER(mcause)    (mcause == 0x000000000000000bull)

extern void OsExcDispatch();        //用户可以使用的异常分发入口函数
extern void hwi_handler();          //用户可以使用的外部中断分发函数
extern void hwi_timer_handler();    //用户可以使用的时钟分发函数
//extern void HwTimerIsr();         //用户需要实现的时钟处理函数，
//形式与注册回调函数的意义是一致的，但是由于时钟中断处理比较重要且必须实现
//用这种形式而不是回调函数的形式，主要是可以利用链接器错误信息告诉用户
//必须实现这个函数
//extern void trap_entry(U64 mcause) //用户需要实现的 trap入口函数


void clear_soft_pending()
{
    *(U32*)CLINT_MSI = 0;
}

//用户实现的样例时钟中断
void HwTimerIsr(void)
{ 
    U64 coreId = PRT_GetCoreID();
    U64 nowTime = *(U64 *)CLINT_TIME;
    *(U64*)CLINT_TIMECMP(coreId) = nowTime + OS_TICK_PER_SECOND;
    PRT_TickISR();
}

//用户实现的trap_entry函数
//对应的中断或异常直接调用内核提供的分发函数即可
//特别的，对于外部中断，我们不需要 [写plic对应的中断声明位claim,这个内核会帮我们在完成处理后自动完成]
//可以拓展的玩法 ：
//  hwi_handler() 只是用来处理 prt_hwi api 注册进去的中断回调函数
//  我们可以在这个基础上扩展自己的回调，在hwi_handler 结束后，执行自己的回调即可
//  但是随时注意， trap_entry 的硬件状态为M态,处于中断当中,在调用 uapi时候需要慎重考虑
void trap_entry(U64 mcause)
{
    if(OS_IS_INTERUPT(mcause)) {
        if(OS_IS_TICK_INT(mcause)) {
            hwi_timer_handler();
        } else if(OS_IS_SOFT_INT(mcause)) {
            clear_soft_pending();
        } else if(OS_IS_EXT_INT(mcause)) {
            hwi_handler();
        }
    } else {
        OsExcDispatch();
    }
}