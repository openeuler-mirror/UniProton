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
 * Description: rpmsglite demo remote/master共用源文件。
 */

#include "rpmsg_common.h"

int rpmsg_init(
    rpmsg_lite_instance_t** local_rpmsg,
    uint32_t linkid,
    void *shmaddr,
    uint32_t flags,
    size_t shmem_length,
    uint8_t type)
{
    rpmsg_lite_instance_t* rpmsg = NULL;
    if (type == MASTER) {
        rpmsg = rpmsg_lite_master_init(
            shmaddr,
            shmem_length,
            linkid,
            flags);
    }
    else {
        rpmsg = rpmsg_lite_remote_init(
            shmaddr,
            linkid, /*normally it should equal to master linkid, but for test it must be different to avoid twice init*/
            flags);
    }
    TEST_ASSERT_MESSAGE(
        NULL != rpmsg,
        "init function failed");

    env_print("Init rpmsg instance success. Instance:0x%lx, linkid:%d, remain shm:%u\n",
                rpmsg,
                rpmsg->link_id,
                rpmsg->sh_mem_remaining);
    *local_rpmsg = rpmsg;
    return RL_SUCCESS;
}

int rpmsg_deinit(
    rpmsg_lite_instance_t* rpmsg
)
{
    int32_t result = rpmsg_lite_deinit(rpmsg);
    TEST_ASSERT_MESSAGE(RL_SUCCESS == result, "deinit function failed");
    TEST_ASSERT_MESSAGE(RL_FALSE == rpmsg_lite_is_link_up(rpmsg),
        "link should be down");
    return RL_SUCCESS;
}

int32_t rpmsg_create_epts(
    rpmsg_lite_instance_t *instance,
    rpmsg_lite_endpoint_t *volatile epts[],
    rl_ept_rx_cb_t ept_cb,
    int32_t count,
    int32_t init_addr,
    ept_rx_cb_data_t* rx_cb_data)
{
    TEST_ASSERT_MESSAGE(epts != NULL, "NULL param");
    TEST_ASSERT_MESSAGE(count > 0, "negative number");
    for (int32_t i = 0; i < count; i++)
    {
        epts[i] = rpmsg_lite_create_ept(instance, init_addr + i, ept_cb, (void*)(rx_cb_data + i));
        TEST_ASSERT_MESSAGE(NULL != epts[i], "'rpmsg_lite_create_ept' failed");
        TEST_ASSERT_MESSAGE(init_addr + i == epts[i]->addr,
                            "'rpmsg_lite_create_ept' does not provide expected address");
        (rx_cb_data + i)->cur_ept = epts[i];
        // env_print("new endpoint:%d, instance:0x%lx, ept:0x%x, addr:%u, cb:0x%lx\n",
            // i, instance, epts[i], init_addr + i, ept_cb);
    }

    return 0;
}

int32_t rpmsg_destory_epts(
    rpmsg_lite_instance_t *instance,
    rpmsg_lite_endpoint_t *volatile epts[],
    int32_t count)
{
    TEST_ASSERT_MESSAGE(epts != NULL, "NULL param");
    TEST_ASSERT_MESSAGE(count > 0, "negative number");
    
    //use different sequence of EP destroy to cover the case when EP is removed from the intermediate element of the EP linked list
    rpmsg_lite_destroy_ept(instance, epts[1]);
    rpmsg_lite_destroy_ept(instance, epts[0]);
    for (int32_t i = 2; i < count; i++)
        rpmsg_lite_destroy_ept(instance, epts[i]);
    return 0;
}