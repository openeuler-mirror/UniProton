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

#include "prt_queue.h"

#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"

#include "rpmsg_common_extern.h"

#ifndef TEST_ASSERT_MESSAGE
#define LOG(log) env_print(log "\n") 
#define TEST_ASSERT_MESSAGE(boolexp, log) if (!(boolexp)) {LOG(log); return 1;}
#endif

#ifndef TEST_ASSERT_MESSAGE_NORET
#define TEST_ASSERT_MESSAGE_NORET(boolexp, log) if (!(boolexp)) {LOG(log);}
#endif

#define MASTER_EPT_INIT_ADDR 0
#define REMOTE_EPT_INIT_ADDR 32
#define RPMSG_NOTIFY_LOCK_USE_INT 1

#if RPMSG_NOTIFY_LOCK_USE_INT == 1
typedef uintptr_t rpmsg_notify_lock_t;
#define CREATE_LOCK(lock) (lock) = 0
#define NOTIFY_RECV(lock) (lock) += 1
#define WAIT_RECV(lock, log) while(!(lock)) {env_sleep_msec(1);} (lock) -= 1
#else
typedef void* rpmsg_notify_lock_t;
#define CREATE_LOCK(lock) env_create_mutex(&lock, 0)
#define NOTIFY_RECV(lock) env_unlock_mutex(lock)
#define WAIT_RECV(lock) env_lock_mutex(lock);
#endif

extern char rpmsg_lite_base[];
extern char rpmsg_lite_end[];
static volatile uintptr_t shmmem_length = 0;
static volatile rpmsg_notify_lock_t node_inner_lock;
static volatile uint32_t current_core;
#define TEST_MSGCOUNT 2

#define RM_LOG_PREFIX "[master]:"
#define RR_LOG_PREFIX "\t[remote]:"
#define RPMSGLITE_LOG(prefix, fmt, ...) env_print(prefix fmt "\n", ##__VA_ARGS__)

#define RPMSG_FAILED() return;

typedef struct rpmsg_lite_instance rpmsg_lite_instance_t;
typedef struct rpmsg_lite_endpoint rpmsg_lite_endpoint_t;
typedef struct rpmsg_lite_ept_static_context rpmsg_lite_ept_static_context_t;

enum {
    MASTER = 0,
    REMOTE
};

struct ept_rx_cb_data {
    rpmsg_lite_endpoint_t* cur_ept;
    struct ept_rx_cb_data* next_cb_data;
    uint32_t data_src;
    uint32_t data_len;
    uint32_t data_payload[RL_BUFFER_PAYLOAD_SIZE(UNI2UNI_RPMSGLITE_TEST_LINKID) / sizeof(uint32_t)];
};
typedef struct ept_rx_cb_data ept_rx_cb_data_t;

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
    int32_t init_addr,
    ept_rx_cb_data_t *rx_cb_data);

int32_t rpmsg_destory_epts(
    rpmsg_lite_instance_t *instance,
    rpmsg_lite_endpoint_t *volatile epts[],
    int32_t count);
#endif