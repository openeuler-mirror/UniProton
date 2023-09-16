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
 * Create: 2023-08-03
 * Description: malloc_usable_size功能实现
 */
#include "stdlib.h"
#include "prt_mem.h"

void *__libc_malloc(size_t n)
{
    return PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, n);
}

static void *default_malloc(size_t n)
{
    return PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, n);
}

weak_alias(default_malloc, malloc);

