/*
 * Copyright (c) 2009-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-4-26
 * Description: rpmsglite demo 模拟remote节点实现。
 */

#include "rpmsg_common.h"

#define REPT_COUNT 32
static rpmsg_lite_endpoint_t *volatile repts[REPT_COUNT] = {0};
static int32_t rtrans_data;
static int32_t rtrans_src;

static void* remote_inner_lock;
#define NOTIFY_RECV(lock) env_unlock_mutex(lock);
#define WAIT_RECV(lock) env_lock_mutex(lock)
/*remote/master rpmsg instance*/
static rpmsg_lite_instance_t* local_rpmsg = NULL;

static rpmsg_lite_endpoint_t* receiver = NULL;
int32_t rept_cb_echo(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    receiver = *((rpmsg_lite_endpoint_t**)priv);
    rtrans_data = *((int32_t *)payload);
    rtrans_src = src;
    env_print("\tremote RECV, data:%x, src:%d, dest:%d\n", rtrans_data, rtrans_src, receiver->addr);
    rtrans_data++;
    NOTIFY_RECV(remote_inner_lock)
    return RL_RELEASE; /* let RPMsg lite know it can free the received data */
}

void rpmsg_remote(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4)
{
    int32_t result;
    uint32_t linkid = 1;
    result = rpmsg_init(
        &local_rpmsg, linkid, rpmsg_lite_base,
        RL_NO_FLAGS, shmmem_length, REMOTE);
    if (result != RL_SUCCESS)
        goto failed;
    /**/
    result = rpmsg_create_epts(local_rpmsg, repts, rept_cb_echo, REPT_COUNT, 0);
    if (result != RL_SUCCESS) 
        goto failed;
    /*create lock between remote recv/send*/
    int res = env_create_mutex(&remote_inner_lock, 0);
    if (res != RL_SUCCESS) {
        env_print("REMOTE inner lock create failed\n");
        goto failed;
    }
    NOTIFY_MASTER();
    env_print("\tREMOTE INIT SUCCESS!\n");

    while (1) {
        WAIT_RECV(remote_inner_lock);
        int32_t result = rpmsg_lite_send(local_rpmsg, receiver, rtrans_src, (char*)(&rtrans_data), sizeof(rtrans_data), RL_BLOCK);
        if (result  == RL_ERR_NO_MEM) {
            env_print("REMOTE:no enough buf\n");
        } else if (RL_SUCCESS != result){
            env_print("REMOTE send failed, err:%d\n", result);
            goto failed;
        }
        env_print("\tremote SEND, data:%x, dest:%d\n", rtrans_data, rtrans_src);
    }

    result = rpmsg_destory_epts(local_rpmsg, repts, REPT_COUNT);
    if (result != 0) {
        goto failed;
    }

failed:
    RPMSG_FAILED();
}
