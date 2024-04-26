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
 * Description: rpmsglite demo remote/master共用头文件。
 */

#ifndef RPMSG_COMMON_H
#define RPMSG_COMMON_H

#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"

#ifndef TEST_ASSERT_MESSAGE
#define LOG(log) env_print(log) 
#define TEST_ASSERT_MESSAGE(boolexp, log) if (!(boolexp)) {LOG(log); return 1;}
#endif

#define WAIT_FOR_REMOTE() env_lock_mutex(env_lock_r2m)
#define WAIT_FOR_MASTER() env_lock_mutex(env_lock_m2r)
#define NOTIFY_REMOTE() env_unlock_mutex(env_lock_m2r)
#define NOTIFY_MASTER() env_unlock_mutex(env_lock_r2m)

#define RPMSG_FAILED() return;

typedef struct rpmsg_lite_instance rpmsg_lite_instance_t;
typedef struct rpmsg_lite_endpoint rpmsg_lite_endpoint_t;
typedef struct rpmsg_lite_ept_static_context rpmsg_lite_ept_static_context_t;

/*notify block for test*/
extern void* env_lock_m2r;
extern void* env_lock_r2m;
/**/
extern char rpmsg_lite_base[];
extern size_t shmmem_length;
/*must be inited to sysclock to make sure env_sleep work properly*/
extern uint64_t current_sys_clock;

enum {
    MASTER = 0,
    REMOTE
};

int rpmsg_init(
    rpmsg_lite_instance_t** local_rpmsg,
    uint32_t linkid,
    void *shmaddr,
    uint32_t flags,
    size_t shmem_length,
    uint8_t type);
int rpmsg_deinit(
    rpmsg_lite_instance_t* rpmsg);
int32_t rpmsg_create_epts(
    rpmsg_lite_instance_t *instance,
    rpmsg_lite_endpoint_t *volatile epts[],
    rl_ept_rx_cb_t ept_cb,
    int32_t count,
    int32_t init_addr);
int32_t rpmsg_destory_epts(
    rpmsg_lite_instance_t *instance,
    rpmsg_lite_endpoint_t *volatile epts[],
    int32_t count);

void rpmsg_master(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
void rpmsg_remote(
    uintptr_t param1, 
    uintptr_t param2, 
    uintptr_t param3, 
    uintptr_t param4);
#endif