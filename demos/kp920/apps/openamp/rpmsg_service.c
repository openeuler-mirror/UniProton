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

#include "rpmsg_backend.h"

static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
struct rpmsg_device *g_rdev;
struct rpmsg_endpoint g_ept;
#if defined(OS_OPTION_OPENAMP_PROXYBASH)
struct rpmsg_endpoint g_proxybash_ept;
#endif
U32 g_receivedMsg;
bool g_openampFlag = false;
#define RPMSG_ENDPOINT_NAME "console"
#if defined(OS_OPTION_OPENAMP_PROXYBASH)
#define PROXYBASH_RPMSG_ENDPOINT_NAME "proxybash"
#endif
void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept, message, len);
}

static int g_s0 = 0;
char *g_s1 = "Hello, UniProton! \r\n";
int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    g_openampFlag = true;
    if (g_s0 == 0) {
        send_message((void *)g_s1, strlen(g_s1) * sizeof(char));
        g_s0++;
    }
    return OS_OK;
}

#if defined(OS_OPTION_OPENAMP_PROXYBASH)

#define PROXYBASH_BUFF_LEN  0x800
bool g_proxybash_openampFlag = false;
char proxybash_result_buff[PROXYBASH_BUFF_LEN];
unsigned int proxybash_result_buff_len = sizeof(proxybash_result_buff);
unsigned int proxybash_result_len = 0;

int proxybash_rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data,
    size_t len, uint32_t src, void *priv)
{
    printf("func:%s line:%d len:%d\r\n", __FUNCTION__, __LINE__, len);
    if (len > 0) {
        proxybash_result_len = len;
        memcpy_s(proxybash_result_buff, proxybash_result_buff_len, data, len);
    }
    g_proxybash_openampFlag = true;
    return OS_OK;
}

int proxybash_send_message(unsigned char *message, int len)
{
    g_proxybash_openampFlag = false;
    proxybash_result_len = 0;
    return rpmsg_send(&g_proxybash_ept, message, len);
}

int proxybash_exec(char *cmdline, char *result_buf, unsigned int buf_len)
{
    int i;
    int ret;
    int retry = 0;
    ret = proxybash_send_message(cmdline, (strlen(cmdline) + 1));
    if (ret < 0) {
        return ret;
    }

    printf("func:%s line:%d\r\n", __FUNCTION__, __LINE__);
    while (g_proxybash_openampFlag == false) {
        PRT_TaskDelay(10);
        if (retry++ > 0x10000) {
            return -1;
        }
    }
    printf("func:%s line:%d retry:%d\r\n", __FUNCTION__, __LINE__, retry);
    if (g_proxybash_openampFlag && proxybash_result_len > 0) {
        printf("proxybash_result_buff(%u):%s\r\n", proxybash_result_len,
            proxybash_result_buff);
        memcpy_s(result_buf, buf_len, proxybash_result_buff,
            proxybash_result_len);
        memset_s(proxybash_result_buff, proxybash_result_buff_len, 0,
            proxybash_result_buff_len);
    }

    /* 增加结束符检查 */
    if (result_buf[proxybash_result_len - 1] != '\0') {
        proxybash_result_len++;
        result_buf[proxybash_result_len - 1] = '\0';
    }
    printf("func:%s line:%d len:%d\r\n", __FUNCTION__, __LINE__, (int)proxybash_result_len);
    return (int)proxybash_result_len;
}
#endif
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

#if defined(OS_OPTION_OPENAMP_PROXYBASH)
    err = rpmsg_create_ept(&g_proxybash_ept, g_rdev,
                           PROXYBASH_RPMSG_ENDPOINT_NAME, 0xE, RPMSG_ADDR_ANY,
                           proxybash_rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (err) {
        return err;
    }
#endif

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

#if defined(OS_OPTION_OPENAMP_PROXYBASH)
    while (!is_rpmsg_ept_ready(&g_proxybash_ept));
#endif

    while (!is_rpmsg_ept_ready(&g_ept));

    return OS_OK;
}
