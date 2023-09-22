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
 * Create: 2023-08-09
 * Description: realloc功能实现
 */
#include "stdlib.h"
#include "prt_mem.h"
#include "../../../mem/fsc/prt_fscmem_internal.h"
#include "malloc_usable_size.h"

void *realloc(void *p, size_t n)
{
    void *newPtr = NULL;
    size_t oldSize;

    if (!p) {
        return malloc(n);
    }
    if (n == 0) {
        free(p);
        return NULL;
    }

    oldSize = malloc_usable_size(p);
    if (oldSize == n) {
        return p;
    }
    newPtr = malloc(n);
    if (!newPtr) {
        return NULL;
    }
    memcpy(newPtr, p, (n < oldSize) ? n : oldSize);
    free(p);
    return newPtr;
}

