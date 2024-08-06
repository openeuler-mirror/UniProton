/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2022-12-05
 * Description: openamp configuration
 */

#ifndef COMMON_H__
#define COMMON_H__

#include "cpu_config.h"

#define VDEV_START_ADDR		MMU_OPENAMP_ADDR
#define VDEV_SIZE		0x30000

#define VDEV_STATUS_ADDR	VDEV_START_ADDR
#define VDEV_STATUS_SIZE	0x4000

#define SHM_START_ADDR		(VDEV_START_ADDR + VDEV_STATUS_SIZE)
#define SHM_SIZE		(VDEV_SIZE - VDEV_STATUS_SIZE)
#define SHM_DEVICE_NAME		"lonely_device"

#define VRING_COUNT		2
#define VRING_RX_ADDRESS	(VDEV_START_ADDR + SHM_SIZE - VDEV_STATUS_SIZE)
#define VRING_TX_ADDRESS	(VDEV_START_ADDR + SHM_SIZE)
#define VRING_ALIGNMENT		4
#define VRING_SIZE		16

#define CONFIG_RPMSG_SERVICE_NUM_ENDPOINTS	1

#define DEFAULT_PAGE_SHIFT	0xffffffffffffffffULL
#define DEFAULT_PAGE_MASK	0xffffffffffffffffULL

#define VIRTQUEUE_ID		1
#define RPMSG_ROLE		RPMSG_REMOTE

#define OS_OPENAMP_NOTIFY_HWI_NUM	OS_HWI_IPI_NO_07
#define OS_OPENAMP_NOTIFY_HWI_PRIO	0

#define BIT(n)	(1 << (n))

OS_SEC_ALW_INLINE INLINE void sys_write32(uint32_t data, uintptr_t addr)
{
    __asm__ volatile ("dmb sy" : : : "memory");
    __asm__ volatile ("str %w0, [%1]" : : "r" (data), "r" (addr));
}

OS_SEC_ALW_INLINE INLINE uint32_t sys_read32(uintptr_t addr)
{
    uint32_t val;
    __asm__ volatile ("ldr %w0, [%1]" : "=r" (val) : "r" (addr));
    __asm__ volatile ("dmb sy" : : : "memory");
    return val;
}

OS_SEC_ALW_INLINE INLINE void sys_write8(uint8_t data, uintptr_t addr)
{
    __asm__ volatile ("dmb sy" : : : "memory");
    __asm__ volatile ("strb %w0, [%1]" : : "r" (data), "r" (addr));
}

OS_SEC_ALW_INLINE INLINE uint8_t sys_read8(uintptr_t addr)
{
    uint8_t val;
    __asm__ volatile ("ldrb %w0, [%1]" : "=r" (val) : "r" (addr));
    __asm__ volatile ("dmb sy" : : : "memory");
    return val;
}

#endif
