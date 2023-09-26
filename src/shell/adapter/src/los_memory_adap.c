/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-25
 * Description: shell los memeory 适配实现。
 */
#include "los_memory.h"
#include "prt_mem_external.h"

VOID *LOS_MemAlloc(VOID *pool, UINT32 size)
{
    return PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, size);
}

UINT32 LOS_MemFree(VOID *pool, VOID *ptr)
{
    return PRT_MemFree(0, ptr);
}