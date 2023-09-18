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
 * Description: memalign功能实现
 */
#include "stdlib.h"
#include "prt_mem.h"
#include <errno.h>

int posix_memalign(void **res, size_t align, size_t len)
{
    if (align < sizeof(void *)) {
        return EINVAL;
    }
    void *mem = aligned_alloc(len, align);
    if (!mem) {
        return errno;
    }
    *res = mem;
    return OS_OK;
}


