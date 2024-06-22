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
 * Description: rpmsglite demo 模拟master节点实现。
 */

#include "rpmsg_common.h"
#include "riscv_ipi.h"
#include "string.h"

#define MEPT_COUNT 8
#define REPT_TEST_ADDR_OFFSET 8
static uint32_t testcount = MEPT_COUNT;
static rpmsg_lite_endpoint_t *volatile mepts[MEPT_COUNT] = {0};
static rpmsg_queue_handle mque[MEPT_COUNT] = {0};
static ept_rx_cb_data_t mept_rx_cb_data[MEPT_COUNT] = {0};

/*remote/master rpmsg instance*/
static rpmsg_lite_instance_t *local_rpmsg = NULL;
static uint32_t ci_success_count = 0;

int32_t mept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    uint32_t val = *(uint32_t *)payload;
    ept_rx_cb_data_t *cb_data = (ept_rx_cb_data_t *)priv;
    TEST_ASSERT_MESSAGE((val == cb_data->data_payload[0] + 1) && (src == cb_data->data_src),
                        "MASTER TEST FAILED\n");
    ci_success_count++;
    NOTIFY_RECV(node_inner_lock);
    return RL_RELEASE; /* let RPMsg lite know it can free the received data */
}

void rpmsg_master(
    uintptr_t param1,
    uintptr_t param2,
    uintptr_t param3,
    uintptr_t param4)
{
    int32_t result;
    uint32_t linkid = UNI2UNI_RPMSGLITE_TEST_LINKID;
    shmmem_length = (uintptr_t)rpmsg_lite_end - (uintptr_t)rpmsg_lite_base;

    /**/
    result = rpmsg_init(
        &local_rpmsg,
        linkid,
        rpmsg_lite_base,
        RL_NO_FLAGS,
        shmmem_length,
        MASTER);
    if (result != RL_SUCCESS)
        goto failed;
    /**/
    result = rpmsg_create_epts(local_rpmsg, mepts, mept_cb, MEPT_COUNT, MASTER_EPT_INIT_ADDR, mept_rx_cb_data);
    if (result != RL_SUCCESS)
        goto failed;

    /*create node inner lock*/
    if (CREATE_LOCK(node_inner_lock) != RL_SUCCESS)
        goto failed;

    /*notify master that remote already initialized, actually this will done
     when rpmsg master init*/
    // virtqueue_kick(local_rpmsg->rvq);
    env_sleep_msec(1000);
    env_print("\tMASTER INIT SUCCESS!\n");

    /**/
    int i = testcount;
    uint32_t data = 0x1234;
    while (i)
    {
        uint32_t src = i & (MEPT_COUNT - 1);
        ept_rx_cb_data_t *cur = &mept_rx_cb_data[src];
        cur->cur_ept = mepts[src];
        cur->data_src = REMOTE_EPT_INIT_ADDR + src;
        cur->data_len = sizeof(data);
        cur->data_payload[0] = data;

        result = rpmsg_lite_send(local_rpmsg, cur->cur_ept, cur->data_src,
                                 (char *)(cur->data_payload), cur->data_len, RL_DONT_BLOCK);
        if (result == RL_ERR_NO_MEM)
        {
            env_print("MASTER:no enough buf\n");
        }
        else if (RL_SUCCESS != result)
        {
            env_print("MASTER send failed, err;%d\n", result);
            goto failed;
        }
        else
        {
            i--;
            data++;
            WAIT_RECV(node_inner_lock, "master loop\n");
        }
    }
    TEST_ASSERT_MESSAGE_NORET(ci_success_count == testcount, "MASTER TEST FAILED");
    env_print("MASTER TEST SUCCESS!\n");
    /**/
    result = rpmsg_destory_epts(local_rpmsg, mepts, MEPT_COUNT);
    if (result != RL_SUCCESS)
    {
        goto failed;
    }
    /**/
    rpmsg_deinit(local_rpmsg);
    return;
failed:
    env_print("MASTER INIT FAILED\n");
    RPMSG_FAILED();
}

static rpmsg_queue_handle ctrl_queue;
static rpmsg_lite_endpoint_t *ctrl_ept;

void rpmsg_master_rpc_on_que(
    uintptr_t param1,
    uintptr_t param2,
    uintptr_t param3,
    uintptr_t param4)
{
    int32_t result;
    uint32_t current_testcount = testcount >> 1;
    uint32_t linkid = UNI2UNI_RPMSGLITE_TEST_LINKID;
    shmmem_length = (uintptr_t)rpmsg_lite_end - (uintptr_t)rpmsg_lite_base;

    /**/
    result = rpmsg_init(
        &local_rpmsg,
        linkid,
        rpmsg_lite_base,
        RL_NO_FLAGS,
        shmmem_length,
        MASTER);
    if (result != RL_SUCCESS)
        goto failed;
    /**/
    TEST_ASSERT_MESSAGE_NORET(
        (ctrl_queue = rpmsg_queue_create(local_rpmsg)) != NULL,
        "Master create ctrl queue failed");
    TEST_ASSERT_MESSAGE_NORET(
        (ctrl_ept = rpmsg_lite_create_ept(local_rpmsg, MASTER_EPT_INIT_ADDR, rpmsg_queue_rx_cb, ctrl_queue)),
        "Master create ctrl ept failed");
    RPMSGLITE_LOG(RM_LOG_PREFIX, "Create ctrl ept&queue suc!");

    for (int32_t i = 0; i < MEPT_COUNT; i++)
    {
        TEST_ASSERT_MESSAGE_NORET(
            ((mque[i] = rpmsg_queue_create(local_rpmsg)) != NULL),
            "Master create que for every ept failed");
        TEST_ASSERT_MESSAGE_NORET(
            (mepts[i] = rpmsg_lite_create_ept(local_rpmsg, RL_ADDR_ANY, rpmsg_queue_rx_cb, mque[i])) != NULL,
            "'rpmsg_lite_create_ept' failed");
        RPMSGLITE_LOG(RM_LOG_PREFIX, "Create new ept&queue suc, addr:%d!", i, mepts[i]->addr);
    }

    /*this has been done when master create rpmsg instance*/
    /*notify master that remote already initialized*/
    // virtqueue_kick(local_rpmsg->rvq);
    /*test env sleep func*/
    env_sleep_msec(1000);

    /*stage 1 test*/
    RPMSGLITE_LOG(RM_LOG_PREFIX, "STAGE1 send test start!");
    int i = current_testcount;
    uint32_t req_create_ept_addr = REMOTE_EPT_INIT_ADDR + REPT_TEST_ADDR_OFFSET;
    /*master send ept create request*/
    while (i)
    {
        uint32_t dst = REMOTE_EPT_INIT_ADDR;
        uint32_t len = sizeof(req_create_ept_addr);

        result = rpmsg_lite_send(local_rpmsg, ctrl_ept, dst,
                                 (char *)&req_create_ept_addr, len, RL_DONT_BLOCK);
        if (result == RL_ERR_NO_MEM)
        {
            env_print("MASTER:no enough buf\n");
            continue;
        }
        else if (RL_SUCCESS != result)
        {
            env_print("MASTER send failed, err;%d\n", result);
            goto failed;
        }
        i--;
        RPMSGLITE_LOG(RM_LOG_PREFIX, "request ept create, dst:%d, req addr:%d", dst, req_create_ept_addr);
        req_create_ept_addr++;
    }
    /*master send create finish msg*/
    req_create_ept_addr = (-1u);
    TEST_ASSERT_MESSAGE_NORET(
        (result = rpmsg_lite_send(local_rpmsg, ctrl_ept, REMOTE_EPT_INIT_ADDR, (char *)&req_create_ept_addr, sizeof(req_create_ept_addr), RL_DONT_BLOCK)) == RL_SUCCESS,
        "Master send create ept finish msg failed");
    RPMSGLITE_LOG(RM_LOG_PREFIX, "STAGE1 send test pass, count:%d!", current_testcount);

    /*master recv ack*/
    RPMSGLITE_LOG(RM_LOG_PREFIX, "STAGE1 recv ACK test start!");
    i = current_testcount;
    while (i)
    {
        uint32_t src = 0;
        uint32_t len = 0;
        char *remote_ack = NULL;
        TEST_ASSERT_MESSAGE_NORET(
            (result = rpmsg_queue_recv_nocopy(local_rpmsg, ctrl_queue, &src, &remote_ack, &len, RL_BLOCK)) == RL_SUCCESS,
            "Master recv ack failed");
        TEST_ASSERT_MESSAGE_NORET(*((uint32_t *)remote_ack) == 1, "Master request failed");
        TEST_ASSERT_MESSAGE_NORET(
            rpmsg_queue_nocopy_free(local_rpmsg, remote_ack) == RL_SUCCESS,
            "Master free ack buffer failed");
        i--;
        RPMSGLITE_LOG(RM_LOG_PREFIX, "check ept create ACK suc, ack:%d!", *((uint32_t *)remote_ack));
    }
    RPMSGLITE_LOG(RM_LOG_PREFIX, "STAGE1 recv ACK test pass, recv times:%d!", current_testcount);


    /*destroy remote env queue*/
    RPMSGLITE_LOG(RM_LOG_PREFIX, "test PASS!!\n");
    result = rpmsg_queue_destroy(local_rpmsg, ctrl_queue);
    for (int i = 0; i < MEPT_COUNT && result == RL_SUCCESS; i++)
        result = rpmsg_queue_destroy(local_rpmsg, mque[i]);
    if (result != RL_SUCCESS)
        env_print("Master rpmsg queue destroy failed\n");
    /*deinit remote rpmsg instance&ept*/
    if ((rpmsg_destory_epts(local_rpmsg, mepts, MEPT_COUNT) != RL_SUCCESS) ||
        (rpmsg_lite_destroy_ept(local_rpmsg, ctrl_ept) != RL_SUCCESS) ||
        (rpmsg_deinit(local_rpmsg) != RL_SUCCESS))
    {
        env_print("Master rpmsg instance&epts destroy failed\n");
        goto failed;
    }
    return;
failed:
    env_print("MASTER TEST FAILED\n");
    RPMSG_FAILED();
}