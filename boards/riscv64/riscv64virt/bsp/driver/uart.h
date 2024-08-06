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
 * Description: uart驱动头文件
 */
#ifndef __UART_H
#define __UART_H
//硬件bsp板载驱动
#include "prt_typedef.h"
#include "platform.h"
#include <stdarg.h>

typedef enum UART_STOP_E {
    STOP_BIT_0  =23u,
    STOP_BIT_0_5    ,
    STOP_BIT_1      ,
} UART_STOP_T;

typedef enum UART_FIFO_SWTCH_E {
    FIFO_EN     =2237u,
    FIFO_DEN
} UART_FIFO_T;

typedef enum UART_INT_E {
    UART_NO_INT         =2366u,
    UART_INT_TX_ONLY          ,
    UART_INT_RX_ONLY          ,
    UART_INT_TX_RX      
} UART_INT_T;

typedef enum UART_CHECK_E {
    UART_PARITY_NONE    =23736u,
    UART_PARITY_EVEN           ,
    UART_PARITY_ODD            ,
} UART_CHECK_T;


//硬件初始化配置参数
typedef struct uart_init_param_s {
    U32             baudrate;
    UART_STOP_T     stop;
    UART_FIFO_T     fifo;
    UART_INT_T      inte;
    UART_CHECK_T    check;
} uart_param_t;


//可以自己写实现
//配置uart io 信息
//void uart_io_ctl(void* param);



//串口硬件初始化
//根据param初始化串口(目前就是固定配置实现，没有根据param来写，可以自己写)
void uart_init(const uart_param_t* param);

//读取数据寄存器内的字符
int uartgetc(void);

//向寄存器内写一个字符
U32  uart_putc(int ch);

//向字符串内写格式化字符串
size_t uart_printf(char* fmt,...);

void uart_putstr_sync(char* fmt);

#endif
