/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-05-13
 * Description: unwind模块的初始化文件
 */
#include "prt_mem_external.h"
#include "prt_unwind_internal.h"

/*
* 描述：OsUnwindInitHdr HDR头。
*/
OS_SEC_L4_TEXT void OsUnwindInitHdrHeadInit(struct OsUnwindHdrHead *header,
                                         const struct OsUnwindTable *table,
                                         uintptr_t fdeCount)
{
    U32 *fde;
    U32 *cie;
    U8 *start;
    U32 hdrEntryCount = 0;
    size_t tableSize = table->size;

    header->headInfo.version = OS_UNWEIND_DEFAULT_VERSION;
    header->headInfo.ehFramePtrEnc = OS_UNWEIND_EH_PE_NATIVE;
    header->headInfo.fdeCountEnc = OS_UNWEIND_EH_PE_NATIVE;
    header->headInfo.tableEnc = OS_UNWEIND_EH_PE_NATIVE;
    header->headInfo.ehFramePtr = table->address;
    header->headInfo.fdeCount = fdeCount;

    for (fde = (U32 *)table->address; tableSize > 0;
         tableSize -= sizeof(*fde) + *fde, fde += (1 + *fde / sizeof(*fde))) {
        cie = (U32 *)OsUnwindGetCieByFde(fde);

        if(fde[1] == OS_UNWEIND_CIE_ID) {
            continue;   /* this is a CIE */
        }

        start = (U8 *)(fde + OS_UNWEIND_FED_HEAD_OFFSET);
        header->table[hdrEntryCount].start = OsUnwindReadPointer(&start, (U8 *)(fde + 1) + *fde, OsUnwindFdePointerType(cie));
        header->table[hdrEntryCount].fde = (uintptr_t)fde;
        ++hdrEntryCount;
    }
    return;
}

/*
* 描述：OsUnwindInit Head Table从小到大排序
*/
OS_SEC_L2_TEXT void OsUnwindHeadTblEntrySort(struct OsUnwindHdrHeadTableEntry *table, U32 hdrEntryCount)
{
    U32 i;
    U32 j;

    for(i = 0; i < hdrEntryCount; ++i) {
        for(j = i + 1; j < hdrEntryCount; ++j) {
            if (table[i].start > table[j].start) {
                /* 根据start从小到大排序 */
                // OsUnwindHeadTblEntrySwap(&table[i], &table[j]);
                struct OsUnwindHdrHeadTableEntry *e1 = &table[i];
                struct OsUnwindHdrHeadTableEntry *e2 = &table[j];
                uintptr_t tempValue;

                tempValue = e1->start;
                e1->start = e2->start;
                e2->start = tempValue;
                tempValue = e1->fde;
                e1->fde = e2->fde;
                e2->fde = tempValue;
            }
        }
    }
    return;
}

OS_SEC_L4_TEXT U32 OsUnwindInitHdr(struct OsUnwindTable *table)
{
    size_t tableSize = table->size;
    size_t hdrSize;
    U32 hdrEntryCount = 0;
    U32 *fde;
    U32 *cie;
    U32 ptrType;

    struct OsUnwindHdrHead *header;

    if (table->header != NULL){
        return OS_FAIL;
    }

    if (OS_NOT_ALIGN_CHK(table->size, sizeof(U32))) {
        return OS_FAIL;
    }

    for (fde = (U32 *)table->address; ((tableSize > sizeof(*fde)) && (tableSize - sizeof(*fde) >=* fde));
         (tableSize -= (sizeof(*fde) + *fde), fde += (1 + *fde / sizeof(*fde)))) {
        cie = (U32 *)OsUnwindGetCieByFde(fde);
        if (cie == &g_unwindNotFde) {
            continue;
        }

        if ((cie == NULL) || (cie == &g_unwindBadCie)) {
            return OS_FAIL;
        }

        ptrType = OsUnwindFdePointerType(cie);
        if (ptrType == OS_UNWEIND_POINTERTYPE_INV) {
            return OS_FAIL;
        }
        ++hdrEntryCount;
    }

    if ((tableSize != 0) || (hdrEntryCount == 0)) {
        return OS_FAIL;
    }

    hdrSize = sizeof(struct OsUnwindHdrHeadBaseInfo) + hdrEntryCount * sizeof(struct OsUnwindHdrHeadTableEntry);

    header = OsMemAlloc(OS_MID_EXC, OS_MEM_DEFAULT_FSC_PT, (U32)hdrSize);
    if (header == NULL) {
        return OS_FAIL;
    }

    if (memset_s(header, hdrSize, 0, hdrSize) != EOK) {
        OS_GOTO_SYS_ERROR();
    }

    /* UnwindHdrHead字段填充 */
    OsUnwindInitHdrHeadInit(header, table, hdrEntryCount);

    OsUnwindHeadTblEntrySort(header->table, hdrEntryCount);

    table->hdrsz = hdrSize;
    table->header = (U8 *)header;
    return OS_OK;
}

/*
* 描述：UnWind初始化
*/
OS_SEC_L4_TEXT U32 OsUnwindInit(void)
{
    g_unwindBaseTable.core.pc = (uintptr_t)(&__os_text_start);
    g_unwindBaseTable.core.range = (uintptr_t)(&__os_text_end) - (uintptr_t)(&__os_text_start);
    g_unwindBaseTable.address = (uintptr_t)(&__os_unwind_table_start);
    g_unwindBaseTable.size = (uintptr_t)(&__os_unwind_table_end) - (uintptr_t)(&__os_unwind_table_start);
    g_unwindBaseTable.hdrsz = 0;
    g_unwindBaseTable.header = NULL;
    g_unwindBaseTable.link = NULL;

    // RegTable初始化 需要初始化lr sp pc三个寄存器的偏移信息。
    uintptr_t offset;
    uintptr_t size;
    U32 i;

    /* 通用寄存器RegTable初始化 */
    for (i = 0; i <= OS_UNWEIND_SP_OFFSET; i++) {
        g_unwindRegTableInfo[i].offs = 0;
        g_unwindRegTableInfo[i].width = sizeof(uintptr_t);
    }

    /* LR寄存器RegTable初始化 */
    offset = OS_UNWEIND_MEMBER_OFFSET_OF(struct OsUnwindFrameInfo, regs.lr);
    size = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.lr);
    g_unwindRegTableInfo[OS_UNWEIND_LR_OFFSET].offs = OS_UNWEIND_TABLE_INFO_OFFS(offset, size);
    g_unwindRegTableInfo[OS_UNWEIND_LR_OFFSET].width = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.lr);

    /* SP寄存器RegTable初始化 */
    offset = OS_UNWEIND_MEMBER_OFFSET_OF(struct OsUnwindFrameInfo, regs.sp);
    size = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.sp);
    g_unwindRegTableInfo[OS_UNWEIND_SP_OFFSET].offs = OS_UNWEIND_TABLE_INFO_OFFS(offset, size);
    g_unwindRegTableInfo[OS_UNWEIND_SP_OFFSET].width = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.sp);

    /* PC寄存器RegTable初始化 */
    offset = OS_UNWEIND_MEMBER_OFFSET_OF(struct OsUnwindFrameInfo, regs.pc);
    size = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.pc);
    g_unwindRegTableInfo[OS_UNWEIND_PC_OFFSET].offs = OS_UNWEIND_TABLE_INFO_OFFS(offset, size);
    g_unwindRegTableInfo[OS_UNWEIND_PC_OFFSET].width = OS_UNWEIND_SIZE_OF_MEMBER(struct OsUnwindFrameInfo, regs.pc);

    return OsUnwindInitHdr(&g_unwindBaseTable);
}