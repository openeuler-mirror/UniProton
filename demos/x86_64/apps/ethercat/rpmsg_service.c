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

#include "rpmsg_backend.h"

#define RPMSG_ENDPOINT_NAME "console"

static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
struct rpmsg_device *g_rdev;

#define RPMSG_VIRTIO_CONSOLE_CONFIG            \
    (&(const struct rpmsg_virtio_config) {     \
        .h2r_buf_size = RPMSG_CONSOLE_BUFFER_SIZE, \
        .r2h_buf_size = RPMSG_CONSOLE_BUFFER_SIZE, \
        .split_shpool = false,             \
})

extern int rpmsg_endpoint_init(struct rpmsg_device *rdev);

extern void example_init();

int openamp_init(void)
{
    int err;

    err = rpmsg_backend_init(&io, &vdev);
    if (err) {
        return err;
    }

    err = rpmsg_init_vdev_with_config(&rvdev, &vdev, NULL, io, NULL, RPMSG_VIRTIO_CONSOLE_CONFIG);
    if (err) {
        return err;
    }

    g_rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);

    err = rpmsg_endpoint_init(g_rdev);

    if (err) {
        return err;
    }

    return OS_OK;
}

void openamp_deinit(void)
{
    rpmsg_deinit_vdev(&rvdev);
    if (io) {
        free(io);
    }
}

int rpmsg_service_init(void)
{
    int err;

    err = openamp_init();
    if (err) {
        return err;
    }

    example_init();
    return OS_OK;
}
