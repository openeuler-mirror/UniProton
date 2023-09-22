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
 * Description: malloc_usable_size功能实现
 */
#include "stdlib.h"
#include "../../../mem/fsc/prt_fscmem_internal.h"

size_t malloc_usable_size(void *p)
{
    struct TagFscMemCtrl *currBlk = NULL; /* 当前内存块指针 */
    U32 *blkTailMagic = NULL;

    if (p == NULL) {
        return 0;
    }

    currBlk = (struct TagFscMemCtrl *)OsMemGetHeadAddr((uintptr_t)p);
    /* 找到内存越界检查魔术字的地址 */
    blkTailMagic = (U32 *)((uintptr_t)currBlk + (uintptr_t)(currBlk->size) - (uintptr_t)OS_FSC_MEM_TAIL_SIZE);
    return (U8*)blkTailMagic - (U8*)p;
}
