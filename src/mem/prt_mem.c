/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 内存基本功能的C文件。
 */
#include "prt_mem_internal.h"

OS_SEC_TEXT void *PRT_MemAlloc(U32 mid, U8 ptNo, U32 size)
{
    void *addr;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    addr = g_memArithAPI.alloc(mid, ptNo, size);
    PRT_HwiRestore(intSave);

    return addr;
}

OS_SEC_TEXT void *PRT_MemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow)
{
    void *addr;
    uintptr_t intSave;

    intSave = PRT_HwiLock();
    addr = g_memArithAPI.allocAlign(mid, ptNo, size, alignPow);
    PRT_HwiRestore(intSave);

    return addr;
}

OS_SEC_TEXT U32 PRT_MemFree(U32 mid, void *addr)
{
    U32 ret;
    uintptr_t intSave;

    (void)mid;
    intSave = PRT_HwiLock();
    ret = g_memArithAPI.free(addr);
    PRT_HwiRestore(intSave);

    return ret;
}
