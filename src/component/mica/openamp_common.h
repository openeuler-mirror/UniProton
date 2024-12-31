/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-15
 * Description: openamp configuration
 */

#ifndef COMMON_H__
#define COMMON_H__

#include "cpu_config.h"
#include "prt_buildef.h"
#if (OS_CPU_TYPE==OS_RV64_MILKVDUOL)
#include "uart.h"
#define PRT_Printf uart_printf
#endif

#define VDEV_START_ADDR		MMU_OPENAMP_ADDR
#define SHM_SIZE		OPENAMP_SHM_SIZE

#define SHM_DEVICE_NAME		"lonely_device"

#define VRING_COUNT		2

#define VRING_RX_ADDRESS        -1  /* allocated by Master processor */
#define VRING_TX_ADDRESS        -1  /* allocated by Master processor */
#define VRING_BUFF_ADDRESS      -1  /* allocated by Master processor */

#define VRING_ALIGNMENT		4
#define VRING_SIZE		16

#define CONFIG_RPMSG_SERVICE_NUM_ENDPOINTS	1

#define DEFAULT_PAGE_SHIFT	0xffffffffffffffffULL
#define DEFAULT_PAGE_MASK	0xffffffffffffffffULL

#define VIRTQUEUE_ID		1
#define RPMSG_ROLE		RPMSG_REMOTE

#define OS_OPENAMP_NOTIFY_HWI_NUM	OS_HWI_IPI_NO_07

#if defined(OS_ARCH_RISCV64)

#define OS_OPENAMP_NOTIFY_HWI_PRIO	3 // CAUSE 0 FOR PLIC IS DISABLE INTERRUPT

#else

#define OS_OPENAMP_NOTIFY_HWI_PRIO	0

#endif


#define BIT(n)	(1 << (n))

#endif
