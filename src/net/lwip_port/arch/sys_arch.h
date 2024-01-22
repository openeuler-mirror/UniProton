/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-24
 * Description: 网络
 */
#ifndef LWIP_PORTING_SYS_ARCH_H
#define LWIP_PORTING_SYS_ARCH_H

#include <stdint.h>

/**
 * Semaphore
 */
typedef uint32_t sys_sem_t;

/**
 * Mutex
 */
typedef sys_sem_t sys_mutex_t;

/**
 * MessageBox
 */
typedef sys_sem_t sys_mbox_t;

/**
 * Protector
 */
typedef void *sys_prot_t;

/**
 * Thread
 */
typedef sys_sem_t sys_thread_t;

#endif /* LWIP_PORTING_SYS_ARCH_H */
