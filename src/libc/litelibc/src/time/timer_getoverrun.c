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
 * Description: timer_getoverrun 相关接口实现
 */
#include <time.h>
#include <errno.h>
#include <limits.h>
#include "prt_timer.h"
#include "prt_posix_internal.h"

int timer_getoverrun(timer_t t)
{
	U32 swtmrId = (U32)t;
	U32 overrun = 0;

	if (PRT_TimerGetOverrun(0, swtmrId, &overrun) != OS_OK) {
		errno = EINVAL;
		return PTHREAD_OP_FAIL;
	}

	if (overrun >= (U8)DELAYTIMER_MAX) {
		overrun = DELAYTIMER_MAX;
	}

	return overrun;
}