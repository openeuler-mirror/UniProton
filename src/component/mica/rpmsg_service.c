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
#include "prt_atomic.h"
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shmsg.h"
#endif
#include "prt_config.h"
#include "../proxy/rpc_internal_model.h"

typedef struct umt_send_msg {
        unsigned long phy_addr;
        int data_len;
} umt_send_msg_t;

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

/* RPMsg umt */
#define RPMSG_UMT_EPT_NAME "rpmsg-umt"
#define OPENAMP_SHM_COPY_SIZE 0x100000 /* 1M */
static SemHandle umt_sem;
static struct rpmsg_endpoint umt_ept;
static struct rpmsg_rcv_msg umt_msg;
static U64 g_umt_send_data_addr;

char *g_s1 = "Hello, UniProton! \r\n";
extern char *g_printf_buffer;

static struct PrtSpinLock g_ttyLock;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

unsigned int is_tty_ready(void)
{
    return is_rpmsg_ept_ready(&tty_ept);
}

int send_message(unsigned char *message, int len)
{
    int ret;
    uintptr_t intSave;

    if (!is_rpmsg_ept_ready(&tty_ept)) {
        return 0;
    }
#if defined(OS_OPTION_SMP)
    intSave = PRT_SplIrqLock(&g_ttyLock);
#endif
    ret = rpmsg_send(&tty_ept, message, len);
#if defined(OS_OPTION_SMP)
    PRT_SplIrqUnlock(&g_ttyLock, intSave);
#endif
    return ret;
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
#ifndef LOSCFG_SHELL_MICA_INPUT
    char tx_buff[512];
#endif
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
            #ifdef LOSCFG_SHELL_MICA_INPUT
                ShellCB *shellCb = OsGetShellCB();
                if (shellCb == NULL) {
                    send_message((void *)g_s1, strlen(g_s1) * sizeof(char));
                } else {
                    for(int i = 0; i < tty_msg.len; i++){
                        char c = tty_data[i];
                        ShellCmdLineParse(c, (pf_OUTPUT)printf, shellCb);
                    }
                }
            #else
                ret = snprintf(tx_buff, 512, "Hello, UniProton! Recv: %s\r\n", tty_data);
                rpmsg_send(&tty_ept, tx_buff, ret);
            #endif
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

static int rpmsg_rx_umt_callback(struct rpmsg_endpoint *ept, void *data,
                   size_t len, uint32_t src, void *priv)
{
    struct rpmsg_rcv_msg *umt_msg = priv;
    umt_send_msg_t *msg;
    int ret = 0;

    rpmsg_hold_rx_buffer(ept, data);
    umt_msg->data = data;
    umt_msg->len = len;
    msg = (umt_send_msg_t *)umt_msg->data;

    if(msg->phy_addr)
        PRT_SemPost(umt_sem);
    else
        rpmsg_release_rx_buffer(ept, data);
    return 0;
}

static void *rpmsg_umt_task(void *arg)
{
    int ret;
    ret = PRT_SemCreate(0, &umt_sem);
    if (ret != OS_OK) {
        PRT_Printf("[openamp] failed to create umt sem\n");
        goto err;
    }
    PRT_Printf("[openamp] umt task started\n");
    umt_ept.priv = &umt_msg;
    ret = rpmsg_create_ept(&umt_ept, rpdev, RPMSG_UMT_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_rx_umt_callback, NULL);
    if (ret != 0) {
        PRT_Printf("[openamp] umt endpoint ret:%d \n", ret);
        goto err;
    }
err:
    pthread_exit(NULL);
}

static void *rpmsg_listen_task(void *arg)
{
    /* Waiting for messages from host */
    while (1) {
        receive_message();
        /* TODO: add lifecycle */
    }
    return NULL;
}

/**
 * @brief 接收来自nrtos的传输数据内容。
 *
 * @param rcv_data 指向接收数据的缓冲区的指针。调用者应确保缓冲区已分配足够的内存。
 * @param data_len 指向接收数据长度的指针。调用者应提供一个整型变量的地址，用于存储接收到的数据长度。
 *
 * @return 返回值表示函数执行结果。
 *             - 0 表示成功接收数据。
 *             - -1 值表示接收失败。
 */
int rcv_data_from_nrtos(void *rcv_data, int *data_len)
{
    umt_send_msg_t *msg;
    static int init = 0;

    PRT_SemPend(umt_sem, OS_WAIT_FOREVER);
    if(umt_msg.len) {
        msg = (umt_send_msg_t *)umt_msg.data;
	    if (!init) {
	        g_umt_send_data_addr = msg->phy_addr + OPENAMP_SHM_COPY_SIZE;
            init ++;
	    }
	    memcpy(rcv_data, (void *)msg->phy_addr, msg->data_len);
	    *data_len = msg->data_len;
        rpmsg_release_rx_buffer(&umt_ept, umt_msg.data);
	    return OS_OK;
    }

    return OS_ERROR;
}

/**
 * @brief 发送数据到nrtos。
 *
 * @param send_data 指向发送数据内容的指针。调用者应确保该缓冲区包含要发送的数据。
 * @param data_len 要发送的数据长度。调用者应提供实际要发送的数据长度。
 *
 * @return int 返回值表示函数执行结果。
 *             - 0 表示成功发送数据。
 *             - -1 表示发送失败。
 */
int send_data_to_nrtos(void *send_data, int data_len)
{
    int ret;
    int rpmsg_data_len;

    if (data_len > OPENAMP_SHM_COPY_SIZE)
	    return OS_ERROR;

    memcpy(g_umt_send_data_addr, send_data, data_len);
    rpmsg_data_len = data_len;
    ret = rpmsg_send(&umt_ept, &rpmsg_data_len, sizeof(int));
    if (ret < 0) {
        PRT_Printf("send_data_to_nrtos failed ret %d data_len %d\n", ret, data_len);
        return OS_ERROR;
    }

    return OS_OK;
}

int rpmsg_service_init(void)
{
    int ret0, ret1, ret2, ret3;
    pthread_attr_t attr;
    pthread_t rpc_thread, tty_thread, listen_thread, umt_thread;

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
    ret2 = pthread_create(&listen_thread, &attr, rpmsg_listen_task, NULL);
    ret3 = pthread_create(&umt_thread, &attr, rpmsg_umt_task, NULL);
    pthread_attr_destroy(&attr);
    if (ret0 != 0 || ret1 != 0 || ret2 != 0 || ret3 != 0) {
        /* If no rpmsg tasks, release the backend. */
        PRT_Printf("[openamp] create task fail, %d/%d/%d/%d\n", ret0, ret1, ret2, ret3);
        goto err;
    }
#if defined(OS_OPTION_SMP)
    ret0 = PRT_SplLockInit(&g_ttyLock);
    if (ret0) {
        PRT_Printf("[openamp] spin lock init fail\n");
        return OS_ERROR;
    }
#endif

    while (!is_rpmsg_ept_ready(&rpc_ept)) {
        PRT_TaskDelay(100);
    }

    while (!is_rpmsg_ept_ready(&umt_ept)) {
        PRT_TaskDelay(100);
    }

    PRT_Printf("[openamp] ept ready\n");

    rpmsg_set_default_ept(&rpc_ept);
    g_printf_buffer = (char *)malloc(PRINTF_BUFFER_LEN);

    return OS_OK;

err:
    rpmsg_backend_remove();
    return OS_ERROR;
}
