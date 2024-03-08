/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-22
 * Description: This file populates resource table for UniProton, for use by the Linux host.
 */

#include "resource_table.h"

#ifndef OS_SEC_RSC_TABLE
#define OS_SEC_RSC_TABLE __attribute__((section(".resource_table")))
#endif

OS_SEC_RSC_TABLE static struct fw_resource_table resource_table = {
    .ver = 1,
    .num = RSC_TABLE_NUM_ENTRY,
    .offset = {
        offsetof(struct fw_resource_table, ept_table),
        offsetof(struct fw_resource_table, vdev),
    },

    .ept_table = {
        .type = RSC_VENDOR_EPT_TABLE,
	.num_of_epts = 0,
    },

    /* Virtio device entry */
    .vdev = {
        RSC_VDEV, VIRTIO_ID_RPMSG, 2, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
        VRING_COUNT, {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    .vring0 = {VRING_TX_ADDRESS, VRING_ALIGNMENT,
                   NUM_RPMSG_BUFF, VRING0_ID, 0},
    .vring1 = {VRING_RX_ADDRESS, VRING_ALIGNMENT,
           NUM_RPMSG_BUFF, VRING1_ID, 0},
};

void rsc_table_get(void **table_ptr, int *length)
{
    *table_ptr = (void *)&resource_table;
    *length = sizeof(resource_table);
}
