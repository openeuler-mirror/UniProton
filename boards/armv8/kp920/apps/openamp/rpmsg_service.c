/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2022-12-05
 * Description: openamp configuration
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_backend.h"
#include "cpu_config.h"
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shmsg.h"
#endif
#ifdef OS_OPTION_PROXY
#include "prt_proxy_ext.h"
#endif

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

typedef enum {
    RPMSG_ADDR_CONSOLE = 0xf,
    RPMSG_ADDR_PROXYBASH,
    RPMSG_ADDR_PROXY,
    RPMSG_ADDR_END,
} RPMSG_ADDR_E;

struct rpmsg_endpoint g_ept;

#define RPMSG_ENDPOINT_NAME "console"

/* PROXYBASH 功能后续整合到 PROXY 中，并删除 */
#if defined(OS_OPTION_OPENAMP_PROXYBASH)
struct rpmsg_endpoint g_proxybash_ept;
#define PROXYBASH_RPMSG_ENDPOINT_NAME "proxybash"
#endif

#if defined(OS_OPTION_PROXY)
struct rpmsg_endpoint g_proxy_ept;
#define PROXY_RPMSG_ENDPOINT_NAME "proxy"
extern char *g_printf_buffer;
extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);
extern int rpmsg_client_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
    uint32_t src, void *priv);
#endif

extern void OsPowerOffSetFlag(void);
void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
    OsPowerOffSetFlag();
}

int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept, message, len);
}
static int g_s0 = 0;
char *g_s1 = "Hello, UniProton! \r\n";
int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    if (g_s0 == 0) {
        send_message((void *)g_s1, strlen(g_s1) * sizeof(char));
        g_s0++;
    }
#ifdef LOSCFG_SHELL_MICA_INPUT
    ShellCB *shellCB = OsGetShellCB();
    if (shellCB != NULL) {
        char c = ((char *)data)[0];
        ShellCmdLineParse(c, (pf_OUTPUT)printf, shellCB);
    }
#endif
    return OS_OK;
}

#if defined(OS_OPTION_OPENAMP_PROXYBASH)

#define PROXYBASH_BUFF_LEN  0x800
bool g_proxybash_openampFlag = false;
char g_proxybash_result_buff[PROXYBASH_BUFF_LEN];
unsigned int g_proxybash_result_buff_len = sizeof(g_proxybash_result_buff);
unsigned int g_proxybash_result_len = 0;

int proxybash_rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data,
    size_t len, uint32_t src, void *priv)
{
    if (len > 0) {
        g_proxybash_result_len = len;
        memcpy_s(g_proxybash_result_buff, g_proxybash_result_buff_len, data, len);
    }
    g_proxybash_openampFlag = true;
    return OS_OK;
}

int proxybash_send_message(unsigned char *message, int len)
{
    g_proxybash_openampFlag = false;
    g_proxybash_result_len = 0;
    return rpmsg_send(&g_proxybash_ept, message, len);
}

int proxybash_exec(char *cmdline, char *result_buf, unsigned int buf_len)
{
    int ret;
    int retry = 0;
    ret = proxybash_send_message(cmdline, (strlen(cmdline) + 1));
    if (ret < 0) {
        return ret;
    }

    while (g_proxybash_openampFlag == false) {
        PRT_TaskDelay(10);
        if (retry++ > 0x10000) {
            return -1;
        }
    }
    if (g_proxybash_openampFlag && g_proxybash_result_len > 0) {
        (void)memcpy_s(result_buf, buf_len, g_proxybash_result_buff,
            g_proxybash_result_len);
        (void)memset_s(g_proxybash_result_buff, g_proxybash_result_buff_len, 0,
            g_proxybash_result_buff_len);
    }

    /* 增加结束符检查 */
    if (result_buf[g_proxybash_result_len - 1] != '\0') {
        g_proxybash_result_len++;
        result_buf[g_proxybash_result_len - 1] = '\0';
    }

    return (int)g_proxybash_result_len;
}

int proxybash_exec_lock(char *cmdline, char *result_buf, unsigned int buf_len)
{
    int ret;
    PRT_TaskLock();
    ret = proxybash_exec(cmdline, result_buf, buf_len);
    PRT_TaskUnlock();
    return ret;
}
#endif
int openamp_init(void)
{
    int32_t err;

    err = rpmsg_backend_init(&io, &vdev);
    if (err) {
        return err;
    }

    err = rpmsg_init_vdev_with_config(&rvdev, &vdev, NULL, io, NULL,
        RPMSG_VIRTIO_CONSOLE_CONFIG);
    if (err) {
        return err;
    }

    g_rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);

#if defined(OS_OPTION_OPENAMP_PROXYBASH)
    err = rpmsg_create_ept(&g_proxybash_ept, g_rdev,
        PROXYBASH_RPMSG_ENDPOINT_NAME, RPMSG_ADDR_PROXYBASH, RPMSG_ADDR_ANY,
        proxybash_rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (err) {
        return err;
    }
#endif

#if defined(OS_OPTION_PROXY)
    err = rpmsg_create_ept(&g_proxy_ept, g_rdev,
        PROXY_RPMSG_ENDPOINT_NAME, RPMSG_ADDR_PROXY, RPMSG_ADDR_ANY,
        rpmsg_client_cb, rpmsg_service_unbind);
    if (err) {
        return err;
    }
#endif

    err = rpmsg_create_ept(&g_ept, g_rdev, RPMSG_ENDPOINT_NAME,
                           RPMSG_ADDR_CONSOLE, RPMSG_ADDR_ANY,
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

#if defined(OS_OPTION_OPENAMP_PROXYBASH)
    while (!is_rpmsg_ept_ready(&g_proxybash_ept));
#endif

#if defined(OS_OPTION_PROXY)
    while (!is_rpmsg_ept_ready(&g_proxy_ept));
    rpmsg_set_default_ept(&g_proxy_ept);
    g_printf_buffer = (char *)malloc(PRINTF_BUFFER_LEN);
#endif

    while (!is_rpmsg_ept_ready(&g_ept));

    return OS_OK;
}
