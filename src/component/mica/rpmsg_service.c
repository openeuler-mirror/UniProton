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
 * Create: 2023-11-20
 * Description: openamp configuration
 */

#include "rpmsg_backend.h"
#include "prt_sem.h"
#include "prt_proxy_ext.h"
#include "pthread.h"
#include "stdio.h"

struct rpmsg_rcv_msg {
    void *data;
    size_t len;
};

static struct rpmsg_device *rpdev;

/* RPMsg rpc */
#define RPMSG_RPC_EPT_NAME "rpmsg-rpc"
static struct rpmsg_endpoint rpc_ept;
extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
               void *data, size_t len,
               uint32_t src, void *priv);
extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

/* RPMsg tty */
#define RPMSG_TTY_EPT_NAME "rpmsg-tty"
static SemHandle tty_sem;
static struct rpmsg_endpoint tty_ept;
static struct rpmsg_rcv_msg tty_msg;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return PRT_ProxyWriteStdOut(message, len);
}

static void *rpmsg_rpc_task(void *arg)
{
    int ret;

    ret = rpmsg_create_ept(&rpc_ept, rpdev, RPMSG_RPC_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_client_cb, rpmsg_service_unbind);
    if (ret != 0) {
        PRT_Printf("[openamp] rpc endpoint ret:%d \n", ret);
        goto err;
    }

    rpmsg_set_default_ept(&rpc_ept);
err:
    pthread_exit(NULL);
}

static int rpmsg_rx_tty_callback(struct rpmsg_endpoint *ept, void *data,
                   size_t len, uint32_t src, void *priv)
{
    struct rpmsg_rcv_msg *tty_msg = priv;

    rpmsg_hold_rx_buffer(ept, data);
    tty_msg->data = data;
    tty_msg->len = len;
    PRT_SemPost(tty_sem);

    return 0;
}

static void *rpmsg_tty_task(void *arg)
{
    int ret;
    char tx_buff[512];
    char *tty_data;

    ret = PRT_SemCreate(0, &tty_sem);
    if (ret != OS_OK) {
        PRT_Printf("[openamp] failed to create tty sem\n");
        goto err;
    }

    PRT_Printf("[openamp] tty task started\n");

    tty_ept.priv = &tty_msg;
    ret = rpmsg_create_ept(&tty_ept, rpdev, RPMSG_TTY_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_rx_tty_callback, NULL);
    if (ret != 0) {
        PRT_Printf("[openamp] tty endpoint ret:%d \n", ret);
        goto err;
    }

    while (tty_ept.addr !=  RPMSG_ADDR_ANY) {
        PRT_SemPend(tty_sem, OS_WAIT_FOREVER);
        if (tty_msg.len) {
            tty_data = (char *)tty_msg.data;
            tty_data[tty_msg.len] = '\0';
            ret = snprintf(tx_buff, 512, "Hello, UniProton! Recv: %s\r\n", tty_data);
            rpmsg_send(&tty_ept, tx_buff, ret);
            rpmsg_release_rx_buffer(&tty_ept, tty_msg.data);
        }
        tty_msg.len = 0;
        tty_msg.data = NULL;

        /* TODO: add lifecycle */
    }
    rpmsg_destroy_ept(&tty_ept);
err:
    pthread_exit(NULL);
}

int rpmsg_service_init(void)
{
    int ret0, ret1;
    pthread_attr_t attr;
    pthread_t rpc_thread, tty_thread;

    /* init rpmsg device */
    rpdev = rpmsg_backend_init();
    if (!rpdev) {
        PRT_Printf("[openamp] failed to init rpmsg device\n");
        return OS_ERROR;
    }

    if (pthread_attr_init(&attr) != 0) {
        PRT_Printf("[openamp] failed to init pthread_attr\n");
        goto err;
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        pthread_attr_destroy(&attr);
        PRT_Printf("[openamp] failed to set pthread_attr\n");
        goto err;
    }

    /* create rpmsg task */
    ret0 = pthread_create(&rpc_thread, &attr, rpmsg_rpc_task, NULL);
    ret1 = pthread_create(&tty_thread, &attr, rpmsg_tty_task, NULL);
    pthread_attr_destroy(&attr);
    if (ret0 != 0 && ret1 != 0) {
        /* If no rpmsg tasks, release the backend. */
        PRT_Printf("[openamp] failed to create rpmsg task, ret: %d/%d\n", ret0, ret1);
        goto err;
    }

    /* Waiting for messages from host */
    while (1) {
        receive_message();
        /* TODO: add lifecycle */
    }

err:
    rpmsg_backend_remove();
    return OS_ERROR;
}
