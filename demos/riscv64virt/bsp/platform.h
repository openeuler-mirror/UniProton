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
 * Description: 平台信息描述
 */
#ifndef __PLATFORM_H
#define __PLATFORM_H


#define UART0 0x10000000L
#define UART0_IRQ 10
#define UART_PRIO 3

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

#define PLIC    0x0c000000L
#define CLINT   0x2000000L

#define CLINT_MSI              (CLINT)
#define CLINT_TIME             (CLINT+0xBFF8)
#define CLINT_TIMECMP(hart_id) (CLINT+0x4000+8*(hart_id))

#endif 