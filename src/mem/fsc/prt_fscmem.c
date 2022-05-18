/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
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
 * Description: Fsc 内存实现
 */
#include "prt_task_external.h"
#include "prt_fscmem_internal.h"

#define OWNER_HWI 0xffff
#define OWNER_SYS 0xeeee
/* 判断初始化内存地址和大小是否为4字节对齐 */
#define OS_MEM_GETBIT(addr) (addr & (U32)(sizeof(U32) - 1))

OS_SEC_BSS struct TagMemFuncLib g_memArithAPI; /* 算法对应API */
OS_SEC_BSS struct TagFscMemCtrl g_fscMemNodeList[OS_FSC_MEM_LAST_IDX];
OS_SEC_BSS U32 g_fscMemBitMap = 1;

OS_SEC_TEXT struct TagFscMemCtrl *OsFscMemSearch(U32 size, U32 *idx)
{
    U32 staIdx;
    struct TagFscMemCtrl *currBlk = NULL;
    struct TagFscMemCtrl *headBlk = NULL;
    
    staIdx = OS_FSC_MEM_SZ2IDX(size);
    *idx = staIdx + 1;

    while (TRUE) {
        *idx = OsGetLmb1((g_fscMemBitMap << *idx) >> *idx);
        if (OS_FSC_MEM_LAST_IDX <= *idx) {
            *idx = staIdx;

            headBlk = &g_fscMemNodeList[*idx];
            currBlk = headBlk->next;

            /* 空闲链表非空 */
            while (currBlk != headBlk) {
                /* 找到可用的内存块 */
                if (OS_FSC_MEM_SZGET(currBlk) >= size) {
                    return currBlk;
                }

                currBlk = currBlk->next;
            }

            OS_REPORT_ERROR(OS_ERRNO_FSCMEM_ALLOC_NO_MEMORY);
            return NULL;
        }

        headBlk = &g_fscMemNodeList[*idx];
        /* 空闲链表为空，清除BitMap标志位 */
        if (headBlk->next == headBlk) {
            g_fscMemBitMap &= ~(OS_FSC_MEM_IDX2BIT(*idx));
        } else {
            break;
        }
    }
    currBlk = headBlk->next;

    return currBlk;
}

OS_SEC_TEXT void *OsFscMemAlloc(U32 mid, U32 size)
{
    U32 idx;
    U32 *blkTailMagic = NULL;
    struct TagFscMemCtrl *plotBlk = NULL;
    struct TagFscMemCtrl *currBlk = NULL;
    struct TagFscMemCtrl *nextBlk = NULL;

    if (size == 0) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_SIZE_ZERO);
        return NULL;
    } else if (size >= ((OS_FSC_MEM_MAXVAL - OS_FSC_MEM_USED_HEAD_SIZE) - OS_FSC_MEM_TAIL_SIZE)) {
        OS_REPORT_ERROR(OS_ERRNO_MEM_ALLOC_SIZETOOLARGE);
        return NULL;
    }

    size = ALIGN(size, OS_FSC_MEM_SIZE_ALIGN) + OS_FSC_MEM_USED_HEAD_SIZE + OS_FSC_MEM_TAIL_SIZE;

    currBlk = OsFscMemSearch(size, &idx);
    if (currBlk == NULL) {
        return NULL;
    }

    /* 找到足够空间的空闲链表，并对其进行分割 */
    if (OS_FSC_MEM_SZGET(currBlk) >= (size + OS_FSC_MEM_MIN_SIZE)) {
        currBlk->size -= (U16)size;

        /* 调整链表 */
        if (idx != OS_FSC_MEM_SZ2IDX(currBlk->size)) {
            OsFscMemDelete(currBlk);
            OsFscMemInsert(currBlk, g_fscMemNodeList, &g_fscMemBitMap);
        }

        plotBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk + (uintptr_t)currBlk->size);
        plotBlk->prevSize = currBlk->size;
        plotBlk->size = (U16)size;

        currBlk = plotBlk;
    } else {
        OsFscMemDelete(currBlk);
    }

    nextBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk + (uintptr_t)currBlk->size);
    nextBlk->prevSize = 0;
    currBlk->next = OS_FSC_MEM_MAGIC_USED;

    /* 设置内存越界检查魔术字 */
    blkTailMagic = (U32 *)((uintptr_t)currBlk + (uintptr_t)currBlk->size - (uintptr_t)OS_FSC_MEM_TAIL_SIZE);
    *blkTailMagic = OS_FSC_MEM_TAIL_MAGIC;

    if (mid == 0) {
        if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) == 0) {
            currBlk->prev = (struct TagFscMemCtrl *)OWNER_SYS;
        } else if ((UNI_FLAG & OS_FLG_HWI_ACTIVE) != 0) {
            currBlk->prev = (struct TagFscMemCtrl *)OWNER_HWI;
        } else {
            currBlk->prev = (struct TagFscMemCtrl *)(uintptr_t)(RUNNING_TASK->taskPid);
        }
    } else {
        currBlk->prev = (struct TagFscMemCtrl *)OWNER_SYS;
    }

    return (void *)((uintptr_t)currBlk + (uintptr_t)OS_FSC_MEM_USED_HEAD_SIZE);
}

OS_SEC_TEXT U32 OsFscMemFree(void *addr)
{
    struct TagFscMemCtrl *prevBlk = NULL; /* 前一内存块指针 */
    struct TagFscMemCtrl *currBlk = NULL; /* 当前内存块指针 */
    struct TagFscMemCtrl *nextBlk = NULL; /* 后一内存块指针 */
    U32 *blkTailMagic = NULL;
    uintptr_t blkSize;

    if (addr == NULL) {
        return OS_ERRNO_MEM_FREE_ADDR_INVALID;
    }

    currBlk = (struct TagFscMemCtrl *)((uintptr_t)addr - (uintptr_t)OS_FSC_MEM_USED_HEAD_SIZE);
    blkSize = currBlk->size;

    if ((currBlk->next != OS_FSC_MEM_MAGIC_USED) || (currBlk->size == 0)) {
        return OS_ERRNO_MEM_FREE_SH_DAMAGED;
    }

    blkTailMagic = (U32 *)((uintptr_t)currBlk + blkSize - (uintptr_t)OS_FSC_MEM_TAIL_SIZE);
    if (*blkTailMagic != OS_FSC_MEM_TAIL_MAGIC) {
        return OS_ERRNO_MEM_OVERWRITE;
    }

    nextBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk + blkSize);

    /* 后一内存块未使用，当前模块释放后与其合并 */
    if (nextBlk->next != OS_FSC_MEM_MAGIC_USED) {
        OsFscMemDelete(nextBlk);

        currBlk->size += nextBlk->size;

        if (memset_s(nextBlk, sizeof(struct TagFscMemCtrl), 0, sizeof(struct TagFscMemCtrl)) != EOK) {
            OS_GOTO_SYS_ERROR1();
        }
    }

    /* 前一内存块未使用，当前内存模块与其合并 */
    if (currBlk->prevSize != 0) {
        prevBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk - (uintptr_t)currBlk->prevSize);
        prevBlk->size += currBlk->size;

        OsFscMemDelete(prevBlk);

        if (memset_s(currBlk, sizeof(struct TagFscMemCtrl), 0, sizeof(struct TagFscMemCtrl)) != EOK) {
            OS_GOTO_SYS_ERROR1();
        }
        currBlk = prevBlk;
    }

    /* 合并后的总内存块插入链表 */
    OsFscMemInsert(currBlk, g_fscMemNodeList, &g_fscMemBitMap);

    nextBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk + (uintptr_t)currBlk->size);
    nextBlk->prevSize = currBlk->size;

    return OS_OK;
}

OS_SEC_TEXT void *OsMemAlloc(enum MoudleId mid, U8 ptNo, U32 size)
{
    (void)ptNo;
    return OsFscMemAlloc((U32)mid, size);
}

OS_SEC_TEXT void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow)
{
    (void)ptNo;
    (void)alignPow;
    return OsFscMemAlloc(mid, size);
}

/*
 * 描述：初始化内存
 */
OS_SEC_TEXT U32 OsFscMemInit(U32 addr, U32 size)
{
    U32 idx;
    struct TagFscMemCtrl *headBlk = NULL;
    struct TagFscMemCtrl *currBlk = NULL;
    struct TagFscMemCtrl *nextBlk = NULL;

    /* 异常判断 */
    if ((void *)(uintptr_t)addr == NULL) {
        return OS_ERRNO_MEM_INITADDR_ISINVALID;
    }

    if (OS_MEM_GETBIT(addr) != 0U) {
        return OS_ERRNO_MEM_INITADDR_INVALID;
    }

    if (OS_MEM_GETBIT(size) != 0U) {
        return OS_ERRNO_MEM_INITSIZE_INVALID;
    }

    if (size < OS_FSC_MEM_USED_HEAD_SIZE) {
        return OS_ERRNO_MEM_PTCREATE_SIZE_ISTOOSMALL;
    }

    if (size > OS_FSC_MEM_MAXVAL) {
        return OS_ERRNO_MEM_PTCREATE_SIZE_ISTOOBIG;
    }

    if (memset_s((void *)(uintptr_t)addr, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    /* 链表初始化，指向自己 */
    headBlk = &g_fscMemNodeList[0];
    for (idx = 0; idx < OS_FSC_MEM_LAST_IDX; idx++, headBlk++) {
        headBlk->prev = headBlk;
        headBlk->next = headBlk;
    }

    size -= OS_FSC_MEM_USED_HEAD_SIZE;

    g_fscMemBitMap |= 1U << (31 - OS_FSC_MEM_LAST_IDX);

    /* 获取索引号 */
    idx = OS_FSC_MEM_SZ2IDX(size);
    g_fscMemBitMap |= OS_FSC_MEM_IDX2BIT(idx);

    /* 挂载链表初始化 */
    headBlk = &g_fscMemNodeList[idx];
    currBlk = (struct TagFscMemCtrl *)(uintptr_t)addr;
    currBlk->next = headBlk;
    currBlk->prevSize = 0;
    currBlk->size = (U16)size;
    currBlk->prev = headBlk;
    headBlk->next = currBlk;
    headBlk->prev = currBlk;

    nextBlk = (struct TagFscMemCtrl *)((uintptr_t)currBlk + (uintptr_t)currBlk->size);
    nextBlk->next = OS_FSC_MEM_MAGIC_USED;
    nextBlk->size = 0;

    g_memArithAPI.alloc = OsFscMemAlloc;
    g_memArithAPI.free = OsFscMemFree;

    g_osMemAlloc = OsMemAlloc;

    return OS_OK;
}
