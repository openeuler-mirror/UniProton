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
 * Description: pthread_barrier_wait 相关接口实现
 */
#include "pthread_impl.h"
#include "prt_typedef.h"
#include "semaphore.h"

int pthread_barrier_wait(pthread_barrier_t *b)
{
	/* Trivial case: count was set at 1 */
	if (b->all_count == 1) {
		return PTHREAD_BARRIER_SERIAL_THREAD;
	}

	pthread_mutex_lock(&b->count_lock);
	if (b->wait_count == b->all_count -1) {
		b->wait_count = 0;
		for (int i = 0; i < b->all_count; i++) {
			sem_post(&b->barrier_sem);
		}
		pthread_mutex_unlock(&b->count_lock);
		return PTHREAD_BARRIER_SERIAL_THREAD;
	}
	b->wait_count++;
	pthread_mutex_unlock(&b->count_lock);
	sem_wait(&b->barrier_sem);

	return 0;
}
