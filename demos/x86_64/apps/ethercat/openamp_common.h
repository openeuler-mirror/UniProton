/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-05
 * Description: openamp configuration
 */

#ifndef COMMON_H__
#define COMMON_H__

#include "cpu_config.h"

#define VDEV_START_ADDR     MMU_OPENAMP_VIRT_ADDR
#define VDEV_SIZE           0x30000

#define VDEV_STATUS_ADDR    VDEV_START_ADDR
#define VDEV_STATUS_SIZE    0x4000

#define SHM_START_PHYS_ADDR (MMU_OPENAMP_PHYS_ADDR + VDEV_STATUS_SIZE)
#define SHM_START_VIRT_ADDR (VDEV_START_ADDR + VDEV_STATUS_SIZE)
#define SHM_SIZE            (VDEV_SIZE - VDEV_STATUS_SIZE)
#define SHM_DEVICE_NAME     "lonely_device"

#define VRING_COUNT         2
#define VRING_RX_ADDRESS    (VDEV_START_ADDR + SHM_SIZE - VDEV_STATUS_SIZE)
#define VRING_TX_ADDRESS    (VDEV_START_ADDR + SHM_SIZE)
#define VRING_ALIGNMENT     4
#define VRING_SIZE          16

#define CONFIG_RPMSG_SERVICE_NUM_ENDPOINTS  1

#define DEFAULT_PAGE_SHIFT  0xffffffffffffffffULL
#define DEFAULT_PAGE_MASK   0xffffffffffffffffULL

#define VIRTQUEUE_ID        1
#define RPMSG_ROLE          RPMSG_REMOTE

#define OS_OPENAMP_NOTIFY_HWI_NUM   0xeb
#define OS_OPENAMP_NOTIFY_HWI_PRIO  0

#define BIT(n)  (1 << (n))

OS_SEC_ALW_INLINE INLINE void sys_write32(uint32_t data, uintptr_t addr)
{
    *(volatile uint32_t *)addr = data;
}

OS_SEC_ALW_INLINE INLINE uint32_t sys_read32(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

OS_SEC_ALW_INLINE INLINE void sys_write8(uint8_t data, uintptr_t addr)
{
    *(volatile uint8_t *)addr = data;
}

OS_SEC_ALW_INLINE INLINE uint8_t sys_read8(uintptr_t addr)
{
    return *(volatile uint8_t *)addr;
}

#endif
