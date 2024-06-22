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
#include "riscv_ipi.h"
#include "prt_hwi.h"

#define REPT_COUNT 16
static rpmsg_lite_endpoint_t *repts[REPT_COUNT] = {0};
static rpmsg_lite_endpoint_t *rctrl_ept = NULL;
static rpmsg_queue_handle rque[REPT_COUNT] = {0};
static rpmsg_queue_handle ctrl_queue = 0;
static ept_rx_cb_data_t rept_rx_cb_data[REPT_COUNT] = {0};
static ept_rx_cb_data_t *g_datalist_head = NULL;

/*remote/master rpmsg instance*/
static rpmsg_lite_instance_t *local_rpmsg = NULL;

int32_t rept_cb_echo(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    ((ept_rx_cb_data_t *)priv)->data_payload[0] = *((int32_t *)payload);
    ((ept_rx_cb_data_t *)priv)->data_src = src;
    ((ept_rx_cb_data_t *)priv)->data_len = payload_len;
    /**/
    ((ept_rx_cb_data_t *)priv)->data_payload[0]++;
    /**/
    uintptr_t int_save = PRT_HwiLock();
    ((ept_rx_cb_data_t *)priv)->next_cb_data = g_datalist_head;
    g_datalist_head = (ept_rx_cb_data_t *)priv;
    PRT_HwiRestore(int_save);
    env_print("\tremote RECV, data:%x, len:%d, src:%d, dest:%d\n",
              g_datalist_head->data_payload[0],
              g_datalist_head->data_len,
              g_datalist_head->data_src,
              g_datalist_head->cur_ept->addr);
    NOTIFY_RECV(node_inner_lock);
    return RL_RELEASE; /* let RPMsg lite know it can free the received data */
}

void rpmsg_remote(
    uintptr_t param1,
    uintptr_t param2,
    uintptr_t param3,
    uintptr_t param4)
{
    int32_t result;
    uint32_t linkid = UNI2UNI_RPMSGLITE_TEST_LINKID;
    shmmem_length = (uintptr_t)rpmsg_lite_end - (uintptr_t)rpmsg_lite_base;

    result = rpmsg_init(
        &local_rpmsg,
        linkid,
        rpmsg_lite_base,
        RL_NO_FLAGS,
        shmmem_length,
        REMOTE);
    if (result != RL_SUCCESS)
        goto failed;
    /**/
    result = rpmsg_create_epts(local_rpmsg, repts, rept_cb_echo, REPT_COUNT, REMOTE_EPT_INIT_ADDR, rept_rx_cb_data);
    if (result != RL_SUCCESS)
        goto failed;
    /*create node inner lock*/
    if (CREATE_LOCK(node_inner_lock) != RL_SUCCESS)
        goto failed;

    /*wait for otherside to finish shmmem init*/
    rpmsg_lite_wait_for_link_up(local_rpmsg, (uint32_t)RL_BLOCK);
    if (1 != rpmsg_lite_is_link_up(local_rpmsg))
    {
        env_print("rpmsg_lite_is_link_up function failed");
        goto failed;
    }
    env_print("\tREMOTE INIT SUCCESS!\n");

    while (1)
    {
        WAIT_RECV(node_inner_lock, "remote loop\n");
        uintptr_t int_save = PRT_HwiLock();
        ept_rx_cb_data_t *cur = g_datalist_head;
        g_datalist_head = cur->next_cb_data;
        PRT_HwiRestore(int_save);

        // env_print("\tremote current data:0x%lx\n", cur);
        // env_print("\tremote SEND, data:%x, len:%d, src:%d, dest:%d\n",
        //     cur->data_payload[0],
        //     cur->data_len,
        //     cur->cur_ept->addr,
        //     cur->data_src);

        int32_t result = rpmsg_lite_send(local_rpmsg, cur->cur_ept, cur->data_src,
                                         (char *)(cur->data_payload), cur->data_len, RL_DONT_BLOCK);
        if (result == RL_ERR_NO_MEM)
        {
            env_print("REMOTE:no enough buf\n");
        }
        else if (RL_SUCCESS != result)
        {
            env_print("REMOTE send failed, err:%d\n", result);
            goto failed;
        }
        /**/
        cur->next_cb_data = NULL;
    }
    result = rpmsg_destory_epts(local_rpmsg, repts, REPT_COUNT);
    if (result != 0)
    {
        goto failed;
    }

failed:
    env_print("REMOTE INIT FAILED\n");
    RPMSG_FAILED();
}

void rpmsg_remote_rpc_on_que(
    uintptr_t param1,
    uintptr_t param2,
    uintptr_t param3,
    uintptr_t param4)
{
    int32_t result;
    uint32_t linkid = UNI2UNI_RPMSGLITE_TEST_LINKID;
    shmmem_length = (uintptr_t)rpmsg_lite_end - (uintptr_t)rpmsg_lite_base;

    result = rpmsg_init(
        &local_rpmsg,
        linkid,
        rpmsg_lite_base,
        RL_NO_FLAGS,
        shmmem_length,
        REMOTE);
    if (result != RL_SUCCESS)
        goto failed;
    /*create ctrl ept&queue of remote*/
    ctrl_queue = rpmsg_queue_create(local_rpmsg);
    TEST_ASSERT_MESSAGE_NORET((void *)ctrl_queue != RL_NULL, "Remote create queue failed");
    rctrl_ept = rpmsg_lite_create_ept(local_rpmsg, REMOTE_EPT_INIT_ADDR, rpmsg_queue_rx_cb, ctrl_queue);
    TEST_ASSERT_MESSAGE_NORET(NULL != rctrl_ept, "'rpmsg_lite_create_ept' failed");
    TEST_ASSERT_MESSAGE_NORET(REMOTE_EPT_INIT_ADDR == rctrl_ept->addr,
                              "'rpmsg_lite_create_ept' does not provide expected address");

    /*wait for otherside to finish shmmem init*/
    rpmsg_lite_wait_for_link_up(local_rpmsg, (uint32_t)RL_BLOCK);
    if (1 != rpmsg_lite_is_link_up(local_rpmsg))
    {
        env_print("rpmsg_lite_is_link_up function failed");
        goto failed;
    }
    RPMSGLITE_LOG(RR_LOG_PREFIX, "init suc!");

    uint32_t src;
    uint32_t len;
    uint32_t ack;
    uint32_t req_create_ept_addr;
    uint32_t requests_times = 0;
    /*remote handle ept creation req*/
    while (1)
    {
        /*remote recv req*/
        TEST_ASSERT_MESSAGE_NORET(
            (result =
                 rpmsg_queue_recv(local_rpmsg, ctrl_queue, &src, (char *)&req_create_ept_addr,
                                  RL_BUFFER_PAYLOAD_SIZE(UNI2UNI_RPMSGLITE_TEST_LINKID),
                                  &len, RL_BLOCK)) == RL_SUCCESS,
            "Remote recv failed");
        if (req_create_ept_addr == (-1u))
            break;
        /*create new ept/que with request ept addr*/
        rque[requests_times] = rpmsg_queue_create(local_rpmsg);
        repts[requests_times] = rpmsg_lite_create_ept(local_rpmsg, req_create_ept_addr, rpmsg_queue_rx_cb, rque[requests_times]);
        TEST_ASSERT_MESSAGE_NORET(ack = (((uintptr_t)rque[requests_times] & (uintptr_t)repts[requests_times]) != 0),
                                  "Remote create new que/ept failed");

        /*acknowledge*/
        result = rpmsg_lite_send(local_rpmsg, repts[requests_times], src, (char *)&ack, sizeof(ack), RL_DONT_BLOCK);
        if (result == RL_ERR_NO_MEM)
            env_print("REMOTE:no enough buf\n");
        else if (RL_SUCCESS != result)
        {
            env_print("REMOTE send failed, err:%d\n", result);
            goto failed;
        }
        requests_times++;
    }


    RPMSGLITE_LOG(RR_LOG_PREFIX, "test PASS!");
    /*destroy remote env queue*/
    result = rpmsg_queue_destroy(local_rpmsg, ctrl_queue);
    for (int i = 0; i < requests_times && result == RL_SUCCESS; i++)
        result = rpmsg_queue_destroy(local_rpmsg, rque[i]);
    if (result != RL_SUCCESS)
        env_print("\tRemote rpmsg queue destroy failed\n");
    /*deinit remote rpmsg instance&ept*/
    if ((rpmsg_destory_epts(local_rpmsg, repts, requests_times) != RL_SUCCESS) ||
        (rpmsg_lite_destroy_ept(local_rpmsg, rctrl_ept) != RL_SUCCESS) ||
        (rpmsg_deinit(local_rpmsg) != RL_SUCCESS))
    {
        env_print("\tRemote rpmsg instance&epts destroy failed\n");
        goto failed;
    }
    return;

failed:
    env_print("REMOTE TEST FAILED\n");
    RPMSG_FAILED();
}