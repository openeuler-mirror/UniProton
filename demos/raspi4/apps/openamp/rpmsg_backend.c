/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2022-12-05
 * Description: openamp backend
 */

#include "rpmsg_backend.h"
#include "metal/device.h"
#include "prt_hwi.h"
#include "prt_sem.h"

static SemHandle msg_sem;
static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;

static metal_phys_addr_t shm_physmap[] = { SHM_START_ADDR };
static struct metal_device shm_device = {
    .name = SHM_DEVICE_NAME,
    .bus = NULL,
    .num_regions = 1,
    {
        {
            .virt       = (void *) SHM_START_ADDR,
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
    uint16_t coreMask = 1;
    OsHwiMcTrigger(coreMask, OS_HWI_IPI_NO_07);
}

const struct virtio_dispatch dispatch = {
    .get_status = virtio_get_status,
    .set_status = virtio_set_status,
    .get_features = virtio_get_features,
    .set_features = virtio_set_features,
    .notify = virtio_notify,
};

static void rpmsg_ipi_handler(void)
{
    PRT_SemPost(msg_sem);
}

void receive_message(void)
{
    U32 ret = PRT_SemPend(msg_sem, OS_WAIT_FOREVER);

    if (ret == OS_OK)
        virtqueue_notification(vq[VIRTQUEUE_ID]);
}

static void rpmsg_ipi_init(void)
{
    g_rpmsg_ipi_handler = rpmsg_ipi_handler;
}

static void free_vq(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (vq[i] != NULL)
            metal_free_memory(vq[i]);
    }
}

/**
 * rpmsg_backend_init - register rpmsg-virtio-device.
 *
 * Init a rpmsg backend based on pre-allocated static resources.
 *
 * Return: pointer of rpmsg_device(rpdev) on success, NULL on failure.
 */
struct rpmsg_device *rpmsg_backend_init(void)
{
    int32_t err;
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    struct metal_device *device;

    rpmsg_ipi_init();

    /* Libmetal setup */
    err = metal_init(&metal_params);
    if (err) {
        PRT_Printf("[openamp] metal_init failed %d\n", err);
        goto cleanup_ipi;
    }

    err = metal_register_generic_device(&shm_device);
    if (err) {
        PRT_Printf("[openamp] Couldn't register shared memory device %d\n", err);
        goto cleanup_metal;
    }

    err = metal_device_open("generic", SHM_DEVICE_NAME, &device);
    if (err) {
        PRT_Printf("[openamp] metal_device_open failed %d\n", err);
        goto cleanup_metal;
    }

    io = metal_device_io_region(device, 0);
    if (!io) {
        PRT_Printf("[openamp] get shared memory io region failed %d\n", err);
        goto cleanup_metal;
    }

    /* Virtqueue setup */
    vq[0] = virtqueue_allocate(VRING_SIZE);
    vq[1] = virtqueue_allocate(VRING_SIZE);
    if (!vq[0] || !vq[1]) {
        PRT_Printf("[openamp] virtqueue_allocate failed %d\n", err);
        goto cleanup_vq;
    }

    rvrings[0].io = io;
    rvrings[0].info.vaddr = (void *)VRING_TX_ADDRESS;
    rvrings[0].info.num_descs = VRING_SIZE;
    rvrings[0].info.align = VRING_ALIGNMENT;
    rvrings[0].vq = vq[0];

    rvrings[1].io = io;
    rvrings[1].info.vaddr = (void *)VRING_RX_ADDRESS;
    rvrings[1].info.num_descs = VRING_SIZE;
    rvrings[1].info.align = VRING_ALIGNMENT;
    rvrings[1].vq = vq[1];

    vdev.role = RPMSG_ROLE;
    vdev.vrings_num = VRING_COUNT;
    vdev.func = &dispatch;
    vdev.vrings_info = &rvrings[0];

    /* setup rvdev */
    err = rpmsg_init_vdev_with_config(&rvdev, &vdev, NULL, io, NULL, RPMSG_VIRTIO_CONSOLE_CONFIG);
    if (err) {
        PRT_Printf("[openamp] rpmsg_init_vdev_with_config failed %d\n", err);
        goto cleanup_vq;
    }

    return rpmsg_virtio_get_rpmsg_device(&rvdev);

cleanup_vq:
    free_vq();
cleanup_metal:
    metal_finish();
cleanup_ipi:
    PRT_SemDelete(msg_sem);
    return NULL;
}

void rpmsg_backend_remove(void)
{
    rpmsg_deinit_vdev(&rvdev);
    free_vq();
    metal_finish();
    PRT_SemDelete(msg_sem);
    /* TODO: disable openamp ipi */
}
