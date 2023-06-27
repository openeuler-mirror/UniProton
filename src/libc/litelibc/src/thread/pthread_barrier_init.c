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
 * Description: pthread_barrier_init 相关接口实现
 */
#include "pthread_impl.h"
#include "errno.h"
#include "semaphore.h"
#include "prt_typedef.h"

int pthread_barrier_init(pthread_barrier_t *restrict b, const pthread_barrierattr_t *restrict a, unsigned count)
{
	if (a != NULL && a->__attr != PTHREAD_PROCESS_PRIVATE) {
		return ENOTSUP;
	}
	if (count == 0) {
		return EINVAL;
	}

	if(sem_init(&b->barrier_sem, PTHREAD_PROCESS_PRIVATE, 0) != OS_OK) {
		return EAGAIN;
	}
	if (pthread_mutex_init(&b->count_lock, NULL) != OS_OK) {
		return EAGAIN;
	}
	
	b->wait_count = 0;
	b->all_count = count;
	b->pshared = PTHREAD_PROCESS_PRIVATE;
	
	return OS_OK;
}
