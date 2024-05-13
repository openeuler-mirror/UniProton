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
#include "prt_unwind_internal.h"
#include "prt_lib_external.h"

OS_SEC_L4_BSS struct OsUnwindTable g_unwindBaseTable;
OS_SEC_L4_BSS U32 g_unwindBadCie;
OS_SEC_L4_BSS U32 g_unwindNotFde;
OS_SEC_L4_BSS struct OsUnwindCfa g_unwindBadCfa;
OS_SEC_L4_BSS struct OsUnwindRegTable g_unwindRegTableInfo[OS_UNWEIND_ARCH_REG_SIZE];

/* 跳过代码对齐 */
OS_SEC_L2_TEXT U64 OsUnwindGetUleb(U8 **current, U8 *end)
{
    return OsGetUleb(current, end);
}

/* 跳过数据对齐 */
OS_SEC_L2_TEXT S64 OsUnwindGetSleb(U8 **current, U8 *end)
{
    return OsGetSleb(current, end);
}

/*
* 描述：根据读取类型的ptrType读取数值
*/
OS_SEC_L2_TEXT U32 OsUnwindReadPointerValueByType(union OsUnwindPtrType *typePtr, U8 *end,
                                               U32 ptrType, uintptr_t *retValue)
{
    uintptr_t value = 0;
    switch (ptrType & OS_UNWEIND_EH_PE_FORM) {
        case OS_UNWEIND_EH_PE_DATA2:
            if (end < (U8 *)(typePtr->u16Ptr + 1)) {
                return OS_FAIL;
            }
            if ((ptrType & OS_UNWEIND_EH_PE_SIGNED) != 0) {
                value = (uintptr_t)OsUnwindGetUnaligned16((U8 *)typePtr->s16Ptr++);
            } else {
                value = (uintptr_t)OsUnwindGetUnaligned16((U8 *)typePtr->u16Ptr++);
            }
            break;
        case OS_UNWEIND_EH_PE_DATA4:
            if (end < (U8 *)(typePtr->u32Ptr + 1)) {
                return OS_FAIL;
            }
            if ((ptrType & OS_UNWEIND_EH_PE_SIGNED) != 0) {
                value = (uintptr_t)OsUnwindGetUnaligned32((U8 *)typePtr->s32Ptr++);
            } else {
                value = (uintptr_t)OsUnwindGetUnaligned32((U8 *)typePtr->u32Ptr++);
            }
            break;
        case OS_UNWEIND_EH_PE_DATA8:
        /* fall through */
        case OS_UNWEIND_EH_PE_NATIVE:
            if (end < (U8 *)(typePtr->ulPtr + 1)) {
                return OS_FAIL;
            }
            value = (uintptr_t)OsUnwindGetUnaligned64((U8 *)typePtr->ulPtr++);
            break;
        case OS_UNWEIND_EH_PE_LEB128:
            value = (((ptrType & OS_UNWEIND_EH_PE_SIGNED) != 0) ?
                    (uintptr_t)OsUnwindGetSleb(&typePtr->u8Ptr, end) : (uintptr_t)OsUnwindGetUleb(&typePtr->u8Ptr, end));
            if ((U8 *)typePtr->u8Ptr > end) {
                return OS_FAIL;
            }
            break;
        default:
            return OS_FAIL;
    }
    *retValue = value;
    return OS_OK;
}

/*
* 描述：根据读取类型的ptrType 读取指定类型长度数值
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindReadPointer(U8 **start, U8 *end, U32 ptrType)
{
    uintptr_t value = 0;
    union OsUnwindPtrType typePtr;

    if((ptrType == OS_UNWEIND_POINTERTYPE_INV) || (ptrType == OS_UNWEIND_EH_PE_OMIT)) {
        return 0;
    }

    typePtr.u8Ptr = *start;
    if (OsUnwindReadPointerValueByType(&typePtr, end, ptrType, &value) != OS_OK) {
        return 0;
    }

    switch (ptrType & OS_UNWEIND_EH_PE_ADJUST) {
        case OS_UNWEIND_EH_PE_ABS:
            break;
        case OS_UNWEIND_EH_PE_PCREL:
            value += (uintptr_t)*start;
            break;
        default:
            return 0;
    }
    if ((ptrType & OS_UNWEIND_EH_PE_INDIRECT) != 0) {
        return 0;
    }

    *start = typePtr.u8Ptr;

    return value;
}
/*
* 描述：寻找指定字符地址
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindMemChr(U8 *sAddr, U8 targetChar, U32 count)
{
    U8 *current = sAddr;
    uintptr_t retAddr = 0;
    
    while((count--) != 0) {
        if(*current++ == targetChar) {
            retAddr = (uintptr_t)(current -1);
            break;
        }
    }
    return retAddr;
}
/*
* 描述：由CIE计算FDE类型
*/
OS_SEC_L2_TEXT U32 OsUnwindFdePointerType(U32 *cie)
{
    U8 *fdeStart = (U8 *)(cie + OS_UNWEIND_FED_HEAD_OFFSET);
    U8 version = *fdeStart;
    U8 *aug;
    U8 *end = (U8 *)(cie + 1) + *cie;
    uintptr_t len;
    U32 ptrType;

    if (*++fdeStart == 0) {
        return OS_UNWEIND_EH_PE_NATIVE;
    }

    /* 检查头上是否是增强大小 */
    if (*fdeStart != 'z') {
        return OS_UNWEIND_POINTERTYPE_INV;
    }

    /* 检查扩展字符串终止是否为空 */
    aug = fdeStart;
    fdeStart = (U8 *)OsUnwindMemChr(aug, 0, (U32)(end - fdeStart));
    if (fdeStart == NULL) {
        return OS_UNWEIND_POINTERTYPE_INV;
    }

    ++fdeStart; /* 跳过终止符 */
    (void)OsUnwindGetUleb(&fdeStart, end);
    (void)OsUnwindGetSleb(&fdeStart, end);

    /* 跳过返回地址列 */
    (version <= OS_UNWEIND_DEFAULT_VERSION) ? ((void)++fdeStart) : ((void)OsUnwindGetUleb(&fdeStart, end));

    len = OsUnwindGetUleb(&fdeStart, end);
    if (OS_ADDR_OVERTURN_CHK(fdeStart, len) || ((fdeStart + len) > end)) {
        return OS_UNWEIND_POINTERTYPE_INV;
    }

    end = fdeStart + len;
    while ((*(++aug)) != 0) {
        if(fdeStart >= end) {
            return OS_UNWEIND_POINTERTYPE_INV;
        }
        switch (*aug) {
            case 'L':
                ++fdeStart;
                break;
            case 'P':
                ptrType = *fdeStart++;
                if ((OsUnwindReadPointer(&fdeStart, end, ptrType) == 0) || fdeStart > end) {
                    return OS_UNWEIND_POINTERTYPE_INV;
                }
                break;
            case 'R':
                return *fdeStart;
            default:
                return OS_UNWEIND_POINTERTYPE_INV;
        }
    }
    return OS_UNWEIND_EH_PE_ABS;
}

/*
* 描述：由FDE求CIE。
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindGetCieByFde(U32 *fde)
{
    U32 *cie = NULL;

    if ((*fde == 0) || ((*fde & (sizeof(*fde) - 1)) != 0)) {
        return (uintptr_t)&g_unwindBadCie;
    }

    if (fde[1] == OS_UNWEIND_CIE_ID) {
        return (uintptr_t)&g_unwindNotFde;
    }

    if ((fde[1] & (sizeof(*fde) - 1)) != 0) {
        return 0; /* 不是一个有效的CIE，直接返回 */
    }

    cie = (U32 *)(fde + 1 - fde[1] / sizeof(*fde));
    if ((*cie <= sizeof(*cie) + OS_UNWEIND_CIE_HEAD_OFFSET) || (*cie >= fde[1] - sizeof(*fde)) ||
        (*cie & (sizeof(*cie) - 1)) || (cie[1] != OS_UNWEIND_CIE_ID)) {
            return 0; /* 不是一个有效的CIE，直接返回 */
    }
    return (uintptr_t)cie;
}

/*
* 描述：根据addr查找地址是否在指定段中
*/
OS_SEC_L2_TEXT bool OsUnwindAddrInSpecifiedSec(uintptr_t addr, uintptr_t startAddr, uintptr_t addrRange)
{
    uintptr_t start = startAddr;
    uintptr_t end = startAddr + addrRange;

    while (start < end) {
        if((addr >= *(uintptr_t *)start) && (addr <= *(uintptr_t *)(start + sizeof(uintptr_t)))) {
            return TRUE;
        }

        start += sizeof(uintptr_t) * OS_UNWEIND_PROCESSS_TEXT_LABEL_PAIR;
    }
    return FALSE;
}

/*
* 描述：根据pc查找UnwindTable。
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindFindTableByPc(uintptr_t pc)
{
    struct OsUnwindTable *table = NULL;
    uintptr_t retTableAddr;
    for (table = &g_unwindBaseTable; table != NULL; table = table->link) {
        if (OsUnwindAddrInSpecifiedSec(pc, table->core.pc, table->core.range)) {
            break;
        }
    }
    retTableAddr = (uintptr_t)table;
    return retTableAddr;
}

/*
* 描述：检查frameInfo pc lr是否落在Text段。
*/
OS_SEC_L2_TEXT bool OsUnwindPcLrInIsTextSec(struct OsUnwindFrameInfo *frameInfo)
{
    uintptr_t textStart = g_unwindBaseTable.core.pc;
    uintptr_t textRange = g_unwindBaseTable.core.range;
    uintptr_t pc = OS_UNWEIND_FRAME_PC(frameInfo);
    uintptr_t lr = OS_UNWEIND_FRAME_LR(frameInfo);
    return OsUnwindAddrInSpecifiedSec(pc, textStart, textRange) && OsUnwindAddrInSpecifiedSec(lr, textStart, textRange);
}