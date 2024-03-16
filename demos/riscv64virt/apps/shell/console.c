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
 * Create: 2024-02-22
 * Description: console相关的代码
 */
#include "uart.h"
#include "prt_typedef.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "prt_sys.h"
#include "platform.h"
#include "prt_hwi.h"
#include "prt_clk.h"
#include "prt_sem.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_buildef.h"
#include "prt_config.h"
#include "console.h"
#include "shell.h"
#include "show.h"
#define RCV_BUF_SIZE  1024
static char rcv_buf[RCV_BUF_SIZE];
static U64  tail;
static U64  head;
static U8   need_process_flag;
SemHandle   buf_sem;
static void ch_in_buf(int ch) 
{
    uintptr_t intSave = PRT_HwiLock();
    if(need_process_flag == 1) {
        PRT_HwiRestore(intSave);
        return;
    }
    // buf full
    if(tail +1 == head) {
        need_process_flag=1;
        PRT_SemPost(buf_sem);
        uart_putc('\n');
        PRT_HwiRestore(intSave);
        return;
    }
    PRT_HwiRestore(intSave);
    if(ch == '\b' || ch =='\177')  {
        uart_putc('\b');
        uart_putc(' ');
        uart_putc('\b');
        if(head!=tail) {
            tail--;
        }
        return;
    }
    if(ch == '\n' || ch == '\r') {
        rcv_buf[tail++] = '\n';
        uart_putc('\n');
        need_process_flag=1;
        PRT_SemPost(buf_sem);
        return;
    }
    rcv_buf[tail++] = ch;
    uart_putc(ch);
    return;    
}

void uart_rcv_handler(HwiArg hwi_id)
{
   while(1) {
    int ch = uartgetc();
    if(ch == -1) {
        break;
    }
    ch_in_buf(ch);
   }
}

void UartPuts(const char *s, U32 len, bool isLock)
{
  for(U32 i=0;i<len;i++)
  {
      uart_putc(s[i]);
  }
}

ssize_t UartRead(int fd, void *str, ssize_t n)
{
    uintptr_t intSave_tmp = PRT_HwiLock();
    if(head == tail)
    {
	PRT_HwiRestore(intSave_tmp);
    	PRT_SemPend(buf_sem, OS_WAIT_FOREVER);
    	if(head == tail) return 0;
    }
    else
    {
	PRT_HwiRestore(intSave_tmp);
    }
    int i =0;
    for(;head!=tail && n>0;head++,i++,n--)
    {
	((char*)str)[i] = rcv_buf[head];
    }
    if(head == tail)
    {
    	uintptr_t intSave = PRT_HwiLock();
    	need_process_flag = 0;
    	PRT_HwiRestore(intSave);
    }
    return i;
}

void console_init(void)
{
    head = 0;
    tail = 0;
    need_process_flag = 0;
    int ret = PRT_SemCreate(0,&buf_sem);
    if(ret != OS_OK) {
        uart_printf("sem init error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiSetAttr(UART0_IRQ, UART_PRIO, OS_HWI_MODE_ENGROSS);
    if(ret != OS_OK) {
        uart_printf("hwiSetAttr error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiCreate(UART0_IRQ, uart_rcv_handler, 0);
    if(ret != OS_OK) {
        uart_printf("hwiSetCreate error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiEnable(UART0_IRQ);
    if(ret != OS_OK) {
        uart_printf("hwiEnable error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

}

