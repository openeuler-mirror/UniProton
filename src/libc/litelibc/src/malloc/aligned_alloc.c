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
 * Create: 2023-08-01
 * Description: aligned_alloc功能实现
 */
#include "stdlib.h"
#include "prt_mem.h"
#include "stdint.h"
#include "errno.h"

void *aligned_alloc(size_t align, size_t len)
{
    int alignPow = 0;

    /* If the alignment is greater than SIZE_MAX / 2 + 1 it cannot be a
       power of 2 and will cause overflow in the check below.  */
    if (align > SIZE_MAX / 2 + 1) {
        errno = EINVAL;
        return NULL;
    }
    while ((1UL << alignPow) < align) {
        alignPow++;
    }
    return PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, len, alignPow);
}

