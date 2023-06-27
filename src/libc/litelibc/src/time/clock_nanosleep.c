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
 * Description: clock_nanosleep 相关接口实现
 */
#include <time.h>
#include <errno.h>
#include "prt_posix_internal.h"
#include "prt_sys_external.h"

int __clock_nanosleep(clockid_t clk, int flags, const struct timespec *req, struct timespec *rem)
{
	struct timespec tmp;
	int ret;
	U64 nanosec;
	U64 now_nanosec;

	if (clk == CLOCK_THREAD_CPUTIME_ID) {
		return EINVAL;
	}
	if (clk != CLOCK_REALTIME) {
		return EINVAL;
	}
	if (!flags) {
		if(nanosleep(req, rem) != OS_OK) {
			return EINVAL;
		}
		return OS_OK;
	}

	ret = clock_gettime(CLOCK_REALTIME, &tmp);
	if (ret != OS_OK) {
		return ret;
	}
	nanosec = (U64)req->tv_sec * OS_SYS_NS_PER_SECOND + (U64)req->tv_nsec;
	now_nanosec = (U64)tmp.tv_sec * OS_SYS_NS_PER_SECOND + (U64)tmp.tv_nsec;
	if (now_nanosec >= nanosec) {
		if (rem != NULL) {
			rem->tv_sec = rem->tv_nsec = 0;
		}
		return OS_OK;
	}
	tmp.tv_sec = (nanosec - now_nanosec) / OS_SYS_NS_PER_SECOND;
	tmp.tv_nsec = (nanosec - now_nanosec) - (tmp.tv_sec * OS_SYS_NS_PER_SECOND);
	if(nanosleep(&tmp, rem) != OS_OK) {
		return EINVAL;
	}
	return OS_OK;
}

weak_alias(__clock_nanosleep, clock_nanosleep);
