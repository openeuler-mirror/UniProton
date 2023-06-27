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
 * Description: munmap实现, 当前不支持mumap接口，此为适配__tz.c
 */
#include <sys/mman.h>

static void dummy(void) { }
weak_alias(dummy, __vm_wait);

int __munmap(void *start, size_t len)
{
	// TODO: 当前不支持mumap接口，此为适配__tz.c
	return -1;
}

weak_alias(__munmap, munmap);
