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
 * Description: openamp backend
 */

#include "rpmsg_backend.h"
#include "metal/device.h"
#include "prt_hwi.h"

extern unsigned long long __openamp_phys_addr;
static metal_phys_addr_t shm_physmap[] = { SHM_START_PHYS_ADDR };
static struct metal_device shm_device = {
    .name = SHM_DEVICE_NAME,
    .bus = NULL,
    .num_regions = 1,
    {
        {
            .virt       = (void *) SHM_START_VIRT_ADDR,
            .physmap    = shm_physmap,
            .size       = SHM_SIZE,
            .page_shift = DEFAULT_PAGE_SHIFT,
            .page_mask  = DEFAULT_PAGE_MASK,
            .mem_flags  = 0,
            .ops        = { NULL },
        },
    },
    .node = { NULL },
    .irq_num = 0,
    .irq_info = NULL
};

static struct virtio_vring_info rvrings[2] = {
    [0] = {
        .info.align = VRING_ALIGNMENT,
    },
    [1] = {
        .info.align = VRING_ALIGNMENT,
    },
};

static struct virtqueue *vq[2];

static unsigned char virtio_get_status(struct virtio_device *vdev)
{
    return sys_read8(VDEV_STATUS_ADDR);
}

static void virtio_set_status(struct virtio_device *vdev, unsigned char status)
{
    sys_write8(status, VDEV_STATUS_ADDR);
}

static uint32_t virtio_get_features(struct virtio_device *vdev)
{
    return BIT(VIRTIO_RPMSG_F_NS);
}

static void virtio_set_features(struct virtio_device *vdev, uint32_t features)
{
}

static void virtio_notify(struct virtqueue *vq)
{
    OsTriggerHwi(OS_OPENAMP_NOTIFY_HWI_NUM, 0, 0);
}

const struct virtio_dispatch dispatch = {
    .get_status = virtio_get_status,
    .set_status = virtio_set_status,
    .get_features = virtio_get_features,
    .set_features = virtio_set_features,
    .notify = virtio_notify,
};

static void rpmsg_hwi_handler(void)
{
    virtqueue_notification(vq[VIRTQUEUE_ID]);
}

static U32 rpmsg_hwi_init(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(OS_OPENAMP_NOTIFY_HWI_NUM, OS_OPENAMP_NOTIFY_HWI_PRIO, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_OPENAMP_NOTIFY_HWI_NUM, (HwiProcFunc)rpmsg_hwi_handler, 0);
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

int rpmsg_backend_init(struct metal_io_region **io, struct virtio_device *vdev)
{
    int32_t                  err;
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    struct metal_device     *device;
    unsigned long long phys_addr = *(unsigned long long *)&__openamp_phys_addr;

    err = rpmsg_hwi_init();
    if (err) {
        return err;
    }

    /* Libmetal setup */
    err = metal_init(&metal_params);
    if (err) {
        return err;
    }

    shm_physmap[0] += phys_addr;
    err = metal_register_generic_device(&shm_device);
    if (err) {
        return err;
    }

    err = metal_device_open("generic", SHM_DEVICE_NAME, &device);
    if (err) {
        return err;
    }

    *io = metal_device_io_region(device, 0);
    if (!*io) {
        return err;
    }

    /* Virtqueue setup */
    vq[0] = virtqueue_allocate(VRING_SIZE);
    if (!vq[0]) {
        return -ENOMEM;
    }

    vq[1] = virtqueue_allocate(VRING_SIZE);
    if (!vq[1]) {
        return -ENOMEM;
    }

    rvrings[0].io = *io;
    rvrings[0].info.vaddr = (void *)VRING_TX_ADDRESS;
    rvrings[0].info.num_descs = VRING_SIZE;
    rvrings[0].info.align = VRING_ALIGNMENT;
    rvrings[0].vq = vq[0];
    
    rvrings[1].io = *io;
    rvrings[1].info.vaddr = (void *)VRING_RX_ADDRESS;
    rvrings[1].info.num_descs = VRING_SIZE;
    rvrings[1].info.align = VRING_ALIGNMENT;
    rvrings[1].vq = vq[1];
    
    vdev->role = RPMSG_ROLE;
    vdev->vrings_num = VRING_COUNT;
    vdev->func = &dispatch;
    vdev->vrings_info = &rvrings[0];

    return 0;
}
