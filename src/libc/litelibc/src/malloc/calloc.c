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
 * Description: calloc功能实现
 */
#include "stdlib.h"
#include "../../../../mem/fsc/prt_fscmem_internal.h"

void *calloc(size_t m, size_t n)
{
    if (n && m > (size_t)-1/n) {
        return 0;
    }
    n *= m;
    void *p = malloc(n);
    if (!p) {
        return p;
    }
    return memset(p, 0, ALIGN(n, OS_FSC_MEM_SIZE_ALIGN));
}


