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
 * Create: 2023-11-17
 * Description: openamp configuration
 */

#include "rpmsg_backend.h"
#include <prt_sem.h>

static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
struct rpmsg_device *g_rdev;
struct rpmsg_endpoint g_ept;
U32 g_receivedMsg;
bool g_openampFlag = false;
#define RPMSG_ENDPOINT_NAME "console"

#define RPMSG_CONSOLE_BUFFER_SIZE 2048
#define RPMSG_VIRTIO_CONSOLE_CONFIG            \
    (&(const struct rpmsg_virtio_config) {     \
        .h2r_buf_size = RPMSG_CONSOLE_BUFFER_SIZE, \
        .r2h_buf_size = RPMSG_CONSOLE_BUFFER_SIZE, \        
        .split_shpool = false,             \
})

void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept, message, len);
}

static volatile int connectAck = 0;
static g_recv[32] = {0};
static SemHandle tty_sem;
int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    char recv[32] = {0};
    if(!connectAck)     //first cb as connect ack
    {
        connectAck = 1;
        return;
    }
    
    int minlen = (len < 31) ? len : 31;
    memcpy(g_recv, data, minlen);
    g_recv[minlen] = '\0';
    PRT_SemPost(tty_sem);
 
    return OS_OK;
}

extern bool g_initFlag;
int openamp_init(void)
{
    int32_t err;

    err = rpmsg_backend_init(&io, &vdev);
    if (err) {
        return err;
    }

    err = rpmsg_init_vdev(&rvdev, &vdev, NULL, io, RPMSG_VIRTIO_CONSOLE_CONFIG);
    if (err) {
        return err;
    }
    g_initFlag = true;
    
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

    PRT_SemCreate(0, &tty_sem);

    err = openamp_init();
    if (err) {
        return err;
    }

    while(!connectAck)
        ;
	
    char str[32] = {"hello, uniproton!"};
    PRT_Printf("start send msg.\n");
    while(1)
    {
        err = send_message(str, strlen(str));
        PRT_SemPend(tty_sem, OS_WAIT_FOREVER);

        PRT_Printf("recv msg: %s\n", g_recv);
        
        sleep(2);
    }

    return OS_OK;
}
