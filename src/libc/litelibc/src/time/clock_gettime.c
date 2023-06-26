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
 * Description: clock_gettime 相关接口实现
 */
#include "time.h"
#include "prt_posix_internal.h"
#include "prt_sys_external.h"
#include "prt_timer.h"
#include "prt_swtmr_external.h"
#include "features.h"

#ifdef VDSO_CGT_SYM

static void *volatile vdso_func;

#ifdef VDSO_CGT32_SYM
static void *volatile vdso_func_32;
static int cgt_time32_wrap(clockid_t clk, struct timespec *ts)
{
	long ts32[2];
	int (*f)(clockid_t, long[2]) =
		(int (*)(clockid_t, long[2]))vdso_func_32;
	int r = f(clk, ts32);
	if (!r) {
		/* Fallback to syscalls if time32 overflowed. Maybe
		 * we lucked out and somehow migrated to a kernel with
		 * time64 syscalls available. */
		if (ts32[0] < 0) {
			a_cas_p(&vdso_func, (void *)cgt_time32_wrap, 0);
			return -ENOSYS;
		}
		ts->tv_sec = ts32[0];
		ts->tv_nsec = ts32[1];
	}
	return r;
}
#endif

static int cgt_init(clockid_t clk, struct timespec *ts)
{
	void *p = __vdsosym(VDSO_CGT_VER, VDSO_CGT_SYM);
#ifdef VDSO_CGT32_SYM
	if (!p) {
		void *q = __vdsosym(VDSO_CGT32_VER, VDSO_CGT32_SYM);
		if (q) {
			a_cas_p(&vdso_func_32, 0, q);
			p = cgt_time32_wrap;
		}
	}
#endif
	int (*f)(clockid_t, struct timespec *) =
		(int (*)(clockid_t, struct timespec *))p;
	a_cas_p(&vdso_func, (void *)cgt_init, p);
	return f ? f(clk, ts) : -ENOSYS;
}

static void *volatile vdso_func = (void *)cgt_init;

#endif

int __clock_gettime(clockid_t clk, struct timespec *ts)
{
    if (ts == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    switch (clk) {
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_MONOTONIC_COARSE:
            OsTimeGetHwTime(ts);
            return OS_OK;
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_COARSE:
		case CLOCK_PROCESS_CPUTIME_ID:
            OsTimeGetRealTime(ts);
            return OS_OK;
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
            errno = ENOTSUP;
            return PTHREAD_OP_FAIL;
        default:
            errno = EINVAL;
            return PTHREAD_OP_FAIL;
    }
}

weak_alias(__clock_gettime, clock_gettime);
