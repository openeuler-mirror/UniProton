/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: malloc free功能实现
 */
#include "prt_posix_internal.h"
#include "prt_mem.h"

void *malloc(size_t size)
{
    return PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, size);
}

void free(void *mem)
{
    U32 ret;

    ret = PRT_MemFree(0, mem);
    if (ret != OS_OK) {
        OsErrRecord(ret);
    }
}
