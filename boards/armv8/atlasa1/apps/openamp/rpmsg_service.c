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
 * Create: 2024-01-23
 * Description: openamp configuration
 */

#include "rpmsg_backend.h"

static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
struct rpmsg_device *g_rdev;
struct rpmsg_endpoint g_ept;
U32 g_receivedMsg;
bool g_openampFlag = false;
#define RPMSG_ENDPOINT_NAME "console"

void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept, message, len);
}

char *g_s1 = "Hello, UniProton! \r\n";
int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    g_openampFlag = true;
    send_message((void *)g_s1, strlen(g_s1) * sizeof(char));

    return OS_OK;
}

int openamp_init(void)
{
    int32_t err;

    err = rpmsg_backend_init(&io, &vdev);
    if (err) {
        return err;
    }

    err = rpmsg_init_vdev(&rvdev, &vdev, NULL, io, NULL);
    if (err) {
        return err;
    }

    g_rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);


    err = rpmsg_create_ept(&g_ept, g_rdev, RPMSG_ENDPOINT_NAME,
                           0xF, RPMSG_ADDR_ANY,
                           rpmsg_endpoint_cb, rpmsg_service_unbind);
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
    int32_t err;
    err = openamp_init();
    if (err) {
        return err;
    }
	
    send_message((void *)&g_receivedMsg, sizeof(U32));
	
    while (!g_openampFlag);
		
    return OS_OK;
}
