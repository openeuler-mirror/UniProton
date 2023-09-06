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
 * Description: 内部用函数声明
 */
#ifndef PRT_POSIX_INTERNAL_H
#define PRT_POSIX_INTERNAL_H

#include <errno.h>
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_task_external.h"
#include "prt_list_external.h"
#include "prt_rwlock_internal.h"

#define PRT_SCHED_FIFO          1
#define PTHREAD_DEFAULT_POLICY  PRT_SCHED_FIFO

#define PTHREAD_DEFAULT_PRIORITY 10
#define PTHREAD_OP_FAIL          (-1)

#define PTHREAD_ATTR_UNINIT 0
#define PTHREAD_ATTR_INIT   1

typedef void * (*prt_pthread_startroutine)(void *);

#define MUTEX_MAGIC 0xEBU

typedef pthread_mutex_t prt_pthread_mutex_t;

extern void (*g_pthread_keys_destor[PTHREAD_KEYS_MAX])(void *);

#define PTHREAD_TERMINATED  2
#define PTHREAD_EXITED      3

extern void PRT_PthreadExit(void *retval);
extern U32 OsTimeSpec2Tick(const struct timespec *tp);
extern U32 OsTimeOut2Ticks(const struct timespec *time, U32 *ticks);
extern void OsTimeGetHwTime(struct timespec *hwTime);
extern void OsTimeSetRealTime(const struct timespec *realTime);
extern void OsTimeGetRealTime(struct timespec *realTime);
extern bool OsTimeCheckSpec(const struct timespec *tp);
extern int OsMutexParamCheck(prt_pthread_mutex_t *mutex);
extern void OsPthreadNotifyParents(struct TagTskCb *tskCb);
extern void OsPthreadRunDestructor(struct TagTskCb *self);
extern int OsCondParamCheck(pthread_cond_t *cond);
extern int PRT_PthreadTimedJoin(TskHandle thread, void **status, U32 timeout);

#endif

