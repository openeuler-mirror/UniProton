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
 * Description: times 相关接口实现
 */
#include <sys/times.h>
#include <limits.h>
#include "prt_sys_external.h"

clock_t times(struct tms *tms)
{
	U64 cycles = OsCurCycleGet64();
	clock_t clock_num = (clock_t)cycles;
	if (cycles > LONG_MAX) {
		clock_num = LONG_MAX;
	}
	if (tms != NULL) {
		tms->tms_utime = clock_num;
		tms->tms_stime = clock_num;
		tms->tms_cutime = clock_num;
		tms->tms_cstime = clock_num;
	}
	
	return clock_num;
}
