/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-08
 * Description: This file populates resource table for UniProton, for use by the Linux host.
 */

#ifndef RESOURCE_TABLE_H__
#define RESOURCE_TABLE_H__

#include <stddef.h>
#include <openamp/rpmsg.h>
#include <openamp/remoteproc.h>
#include "openamp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VDEV_ID                 0xFF
#define VRING0_ID 0 /* (master to remote) fixed to 0 for Linux compatibility */
#define VRING1_ID 1 /* (remote to master) fixed to 1 for Linux compatibility */

#define RPMSG_IPU_C0_FEATURES   1
#define NUM_RPMSG_BUFF          8

#define VIRTIO_ID_RPMSG         7
#define RSC_VENDOR_EPT_TABLE    128 /* List of bound endpoints */

enum rsc_table_entries {
    RSC_TABLE_EPT_TABLE_ENTRY,
    RSC_TABLE_VDEV_ENTRY,
    RSC_TABLE_NUM_ENTRY
};

METAL_PACKED_BEGIN
struct ept_info {
	char name[RPMSG_NAME_SIZE];
	uint32_t addr;
	uint32_t dest_addr;
} METAL_PACKED_END;

#define MAX_NUM_OF_EPTS 64

METAL_PACKED_BEGIN
struct fw_rsc_ept {
	uint32_t type;
	uint32_t num_of_epts;
	struct ept_info endpoints[MAX_NUM_OF_EPTS];
} METAL_PACKED_END;


METAL_PACKED_BEGIN
struct fw_resource_table {
    unsigned int ver;
    unsigned int num;
    unsigned int reserved[2];
    unsigned int offset[RSC_TABLE_NUM_ENTRY];

    struct fw_rsc_ept ept_table;
    struct fw_rsc_vdev vdev;
    struct fw_rsc_vdev_vring vring0;
    struct fw_rsc_vdev_vring vring1;
} METAL_PACKED_END;

void rsc_table_get(void **table_ptr, int *length);

#ifdef __cplusplus
}
#endif

#endif
