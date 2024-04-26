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

#define MEPT_COUNT 16
static uint32_t testcount = MEPT_COUNT;
static rpmsg_lite_endpoint_t *volatile mepts[MEPT_COUNT] = {0};
static int32_t mtrans_data;
static int32_t mtrans_src;

/*remote/master rpmsg instance*/
static rpmsg_lite_instance_t* local_rpmsg = NULL;

int32_t mept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    mtrans_data = *((int32_t *)payload);
    mtrans_src = src;
    env_print("master RECV, data:%x, src:%d\n", mtrans_data, mtrans_src);

    return RL_RELEASE; /* let RPMsg lite know it can free the received data */
}

void rpmsg_master(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4)
{
    int32_t result;
    uint32_t linkid = 0;

    /**/
    result = rpmsg_init(
        &local_rpmsg, linkid, rpmsg_lite_base,
        RL_NO_FLAGS, shmmem_length, MASTER);
    if (result != RL_SUCCESS)
        goto failed;
    /**/
    result = rpmsg_create_epts(local_rpmsg, mepts, mept_cb, MEPT_COUNT, 0);
    if (result != RL_SUCCESS)
        goto failed;

    /*wait for master to finish shmmem init*/
    rpmsg_lite_wait_for_link_up(local_rpmsg, (uint32_t)RL_BLOCK);
    if (1 != rpmsg_lite_is_link_up(local_rpmsg)) {
        env_print("rpmsg_lite_is_link_up function failed");
        goto failed;
    }
    env_sleep_msec(1000);
    env_print("\tMASTER INIT SUCCESS!\n");
    
    /**/
    int i = testcount;
    uint32_t data = 0x1234;
    while (i--) {
        result = rpmsg_lite_send(local_rpmsg, mepts[i & (MEPT_COUNT - 1)], i & (MEPT_COUNT - 1),
                                 (char *)(&data), sizeof(data), RL_BLOCK);
        if (result  == RL_ERR_NO_MEM) {
            env_print("MASTER:no enough buf\n");
        } else if (RL_SUCCESS != result){
            env_print("MASTER send failed, err;%d\n", result);
            goto failed;
        }
        env_print("master SEND, data:%x, dest:%d\n", data, i & (MEPT_COUNT - 1));
        data++;
    }
    /**/
    result = rpmsg_destory_epts(local_rpmsg, mepts, MEPT_COUNT);
    if (result != RL_SUCCESS) {
        goto failed;
    }

    /**/
    rpmsg_deinit(local_rpmsg);
    return;
failed:
    RPMSG_FAILED();
}