/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread 相关定义
 */
#if defined(__NEED_pthread_mutexattr_t) && !defined(__DEFINED_pthread_mutexattr_t)
typedef struct __pthread_mutexattr_s {
    unsigned char protocol;
    unsigned char prioceiling;
    unsigned char type;
    unsigned char reserved;
} pthread_mutexattr_t;
#define __DEFINED_pthread_mutexattr_t
#endif  /* end defined(__NEED_pthread_mutexattr_t) */

#if defined(__NEED_pthread_mutex_t) && !defined(__DEFINED_pthread_mutex_t)
typedef struct __pthread_mutex_s {
    unsigned char type;
    unsigned char magic;
    unsigned short mutex_sem;
} pthread_mutex_t;
#define __DEFINED_pthread_mutex_t

#if defined(__NEED_mtx_t) && !defined(__DEFINED_mtx_t)
typedef pthread_mutex_t mtx_t;
#define __DEFINED_mtx_t
#endif  /* defined(__NEED_mtx_t) */

#if defined(__NEED_pthread_condattr_t) && !defined(__DEFINED_pthread_condattr_t)
#include "time.h"
typedef struct __pthread_condattr_s {
    clockid_t clock;
} pthread_condattr_t;
#define __DEFINED_pthread_condattr_t

#if defined(__NEED_pthread_cond_t) && !defined(__DEFINED_pthread_cond_t)
#include "list_types.h"
typedef struct __pthread_cond_s {
    pthread_condattr_t condAttr;
    unsigned int eventMask;
    pthread_mutex_t mutex;
    struct TagListObject head;
} pthread_cond_t;
#define __DEFINED_pthread_cond_t

#if defined(__NEED_cnd_t) && !defined(__DEFINED_cnd_t)
typedef pthread_cond_t cnd_t;
#define __DEFINED_cnd_t
#endif  /* defined(__NEED_cnd_t) */
#endif  /* defined(__NEED_pthread_cond_t) */
#endif  /* defined(__NEED_pthread_condattr_t) */
#endif  /* defined(__NEED_pthread_mutex_t) */

#if defined(__NEED_pthread_attr_t) && !defined(__DEFINED_pthread_attr_t)
#include "sched.h"

typedef struct __pthread_attr_s {
    unsigned int detachstate;
    unsigned int schedpolicy;
    struct sched_param schedparam;
    unsigned int inheritsched;
    unsigned int scope;
    unsigned int stackaddr_set;
    void *stackaddr;
    unsigned int stacksize_set;
    size_t stacksize;
} pthread_attr_t;
#define __DEFINED_pthread_attr_t
#endif  /* defined(__NEED_pthread_attr_t) */

#if defined(__NEED_pthread_rwlock_t) && !defined(__DEFINED_pthread_rwlock_t)
#include "list_types.h"
#include "prt_buildef.h"
#define __NEED_uintptr_t
#include <bits/alltypes.h>

typedef struct __pthread_rwlock_s {
    unsigned int rw_magic : 16;
    unsigned int index : 16;
    int rw_count;
    void *rw_owner;
    struct __pthread_rwlock_s *next;
    struct TagListObject rw_write;
    struct TagListObject rw_read;
#if defined(OS_OPTION_SMP)
    volatile uintptr_t rwSpinLock;
#endif
} pthread_rwlock_t;
#define __DEFINED_pthread_rwlock_t
#endif  /* defined(__NEED_pthread_rwlock_t) */

#if defined(__NEED_pthread_barrier_t) && !defined(__DEFINED_pthread_barrier_t)
#include "semaphore_types.h"

typedef struct __pthread_barrier_s {
    unsigned all_count;
    unsigned wait_count;
    pthread_mutex_t count_lock;
    unsigned char pshared;
    sem_t barrier_sem;
} pthread_barrier_t;
#define __DEFINED_pthread_barrier_t
#endif  /* defined(__NEED_pthread_barrier_t) */
