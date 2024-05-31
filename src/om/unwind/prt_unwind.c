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
 * Create: 2024-3-20
 * Description: unwind模块的初始化文件
 */
#include "prt_mem.h"
#include "prt_unwind_internal.h"
#include "prt_task_external.h"
#include "prt_kexc_external.h"

OS_SEC_L4_BSS bool g_unwindInitFlag;

#if defined(OS_OPTION_SMP)
OS_SEC_L4_BSS volatile uintptr_t g_unwindSplAddr;
#endif

/*
* 描述：OsUnwindAdvanceLoc 计算loc字段
*/
OS_SEC_L2_TEXT bool OsUnwindAdvanceLoc(uintptr_t delta, struct OsUnwindState *state)
{
    state->loc += delta * state->codeAlign;
    return TRUE;
}

/*
* 描述：设置rule 包括where和value
*/
OS_SEC_L2_TEXT void OsUnwindSetRule(uintptr_t reg, enum UnwindItemLocation where,
                                 uintptr_t value, struct OsUnwindState *state)
{
    if (reg < OS_UNWEIND_ARRAY_SIZE(state->regs)) {
        state->regs[reg].where = where;
        state->regs[reg].value = value;
    }
    return;
}

/*
* 描述：解析High 2Bits为零时低6比特[0x0-0x9]指令含义
*/
OS_SEC_L2_TEXT U64 OsUnwindProcessCfiOpcodeFirst(union OsUnwindPtrType *curPtr, U8 *end,
                                              U32 ptrType, struct OsUnwindState *state)
{
    uintptr_t value;
    U64 result = 1ULL;
    U8 opcode = *(curPtr->u8Ptr++);
    switch (opcode) {
        case OS_UNWEIND_CFA_NOP:
            break;
        case OS_UNWEIND_CFA_SET_LOC:
            state->loc = OsUnwindReadPointer(&curPtr->u8Ptr, end, ptrType);
            if (state->loc == 0) {
                result = 0;
            }
            break;
        case OS_UNWEIND_CFA_ADVANCE_LOC1:
            result = (U64)((curPtr->u8Ptr < end) && OsUnwindAdvanceLoc(*curPtr->u8Ptr++, state));
            break;
        case OS_UNWEIND_CFA_ADVANCE_LOC2:
            value = *(curPtr->u8Ptr++);
            value += (((uintptr_t)(*(curPtr->u8Ptr++))) << OS_BITS_PER_BYTE);
            result = (U64)((curPtr->u8Ptr <= (end + sizeof(U16))) && OsUnwindAdvanceLoc(value, state));
            break;
        case OS_UNWEIND_CFA_ADVANCE_LOC4:
            result = (U64)((curPtr->u8Ptr <= (end + sizeof(U32))) && OsUnwindAdvanceLoc(*curPtr->u32Ptr++, state));
            break;
        case OS_UNWEIND_CFA_OFFSET_EXTENDED:
            value = (U32)OsUnwindGetUleb(&curPtr->u8Ptr, end);
            OsUnwindSetRule(value, OS_UNWEIND_MEMORY, OsUnwindGetUleb(&curPtr->u8Ptr, end), state);
            break;
        case OS_UNWEIND_CFA_RESTORE_EXTENDED:
        /* fall through */
        case OS_UNWEIND_CFA_UNDEFINED:
        /* fall through */
        case OS_UNWEIND_CFA_SAME_VALUE:
            OsUnwindSetRule(OsUnwindGetUleb(&curPtr->u8Ptr, end), OS_UNWEIND_NOWHERE, 0, state);
            break;
        case OS_UNWEIND_CFA_REGISTER:
            value = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            OsUnwindSetRule(value, OS_UNWEIND_REGISTER, OsUnwindGetUleb(&curPtr->u8Ptr, end), state);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}
/*
* 描述：解析High 2Bits为零时低6比特[0xA-0xB]指令含义
*/
OS_SEC_L2_TEXT U64 OsUnwindProcessCfiOpcodeSecond(union OsUnwindPtrType *curPtr, U8 *start, U8 *end,
                                               U32 ptrType, struct OsUnwindState *state)
{
    uintptr_t loc;
    U64 result = 1ULL;
    U8 *label = NULL;
    U8 opcode = *(curPtr->u8Ptr++);
    switch (opcode) {
        case OS_UNWEIND_CFA_REMEMBER_STATE:
            if (curPtr->u8Ptr == state->label) {
                state->label = NULL;
                return (1ULL | OS_UNWEIND_PROCESSS_DIRECT_RETURN);
            }
            if (state->stackDepth >= OS_UNWEIND_MAX_STACK_DEPTH) {
                return OS_UNWEIND_PROCESSS_DIRECT_RETURN;
            }
            state->stack[state->stackDepth++] = curPtr->u8Ptr;
            break;
        case OS_UNWEIND_CFA_RESTORE_STATE:
            if (state->stackDepth != 0) {
                loc = state->loc;
                label = state->label;

                state->label = state->stack[state->stackDepth - 1];
                if (memcpy_s(&(state->cfa), sizeof(state->cfa), &g_unwindBadCfa, sizeof(g_unwindBadCfa)) != EOK) {
                    OS_GOTO_SYS_ERROR();
                }
                if (memset_s(state->regs, sizeof(state->regs), 0, sizeof(state->regs)) != EOK) {
                    OS_GOTO_SYS_ERROR();
                }
                state->stackDepth = 0;
                result = OsUnwindProcessCfi(start, end, 0, ptrType, state);
                state->loc = loc;
                state->label = label;
            } else {
                return OS_UNWEIND_PROCESSS_DIRECT_RETURN;
            }
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

/*
* 描述：解析High 2Bits为零时低6比特[0xC-0x2F]指令含义
*/
OS_SEC_L2_TEXT U64 OsUnwindProcessCfiOpcodeThird(union OsUnwindPtrType *curPtr,
                                              U8 *end, struct OsUnwindState *state)
{
    uintptr_t value;
    U64 result = 1ULL;
    U8 opcode = *(curPtr->u8Ptr++);
    switch (opcode) {
        case OS_UNWEIND_CFA_VAL_OFFSET:
            value = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            OsUnwindSetRule(value, OS_UNWEIND_VALUE, OsUnwindGetUleb(&curPtr->u8Ptr, end), state);
            break;
        case OS_UNWEIND_CFA_OFFSET_EXTENDED_SF:
            value = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            OsUnwindSetRule(value, OS_UNWEIND_MEMORY, (uintptr_t)OsUnwindGetSleb(&curPtr->u8Ptr, end), state);
            break;
        case OS_UNWEIND_CFA_VAL_OFFSET_SF:
            value = OsUnwindGetUleb(&curPtr->u8Ptr,end);
            OsUnwindSetRule(value, OS_UNWEIND_VALUE, (uintptr_t)OsUnwindGetSleb(&curPtr->u8Ptr, end), state);
            break;
        case OS_UNWEIND_CFA_DEF_CFA:
            state->cfa.reg = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            state->cfa.offs = OsUnwindGetUleb(&curPtr->u8Ptr,end);
            break;
        case OS_UNWEIND_CFA_DEF_CFA_OFFSET:
            state->cfa.offs = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            break;
        case OS_UNWEIND_CFA_DEF_CFA_SF:
            state->cfa.reg = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            state->cfa.offs = (U64)OsUnwindGetSleb(&curPtr->u8Ptr, end) * state->dataAlign;
            break;
        case OS_UNWEIND_CFA_DEF_CFA_OFFSET_SF:
            state->cfa.offs = (U64)OsUnwindGetSleb(&curPtr->u8Ptr, end) * state->dataAlign;
            break;
        case OS_UNWEIND_CFA_DEF_CFA_REGISTER:
            state->cfa.reg = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            break;
        case OS_UNWEIND_CFA_GNU_ARGS_SIZE:
            (void)OsUnwindGetUleb(&curPtr->u8Ptr, end);
            break;
        case OS_UNWEIND_CFA_GNU_NEGATIVE_OFFSET_EXTENDED:
            value = OsUnwindGetUleb(&curPtr->u8Ptr, end);
            OsUnwindSetRule(value, OS_UNWEIND_MEMORY, (uintptr_t)(-(S64)OsUnwindGetUleb(&curPtr->u8Ptr, end)), state);
            break;
        case OS_UNWEIND_CFA_GNU_WINDOW_SAVE:
        /* fall through */
        default:
            result = 0;
            break;
    }
    return result;
}

/*
* 描述：解析High 2Bits为零时低6比特指令含义
*/
OS_SEC_L2_TEXT U64 OsUnwindProcessCfiOpcode(union OsUnwindPtrType *curPtr, U8 *start, U8 *end,
                                         U32 ptrType, struct OsUnwindState *state)
{
    U64 result = 1ULL;
    U8 opcode = *(curPtr->u8Ptr);
    /* 在High 2Bits 为零时instructions Opcode解析字段含义特别长，分三段进行处理 */
    if (opcode <= OS_UNWEIND_CFA_REGISTER) {
        /* 1、操作码low 6Bits在[0x0-0x9]的 */
        result = (U64)OsUnwindProcessCfiOpcodeFirst(curPtr, end, ptrType, state);
    } else if ((opcode >= OS_UNWEIND_CFA_REMEMBER_STATE) && (opcode <= OS_UNWEIND_CFA_RESTORE_STATE)) {
        /* 2、操作码low 6Bits在[0xA-0xB]的 */
        result = (U64)OsUnwindProcessCfiOpcodeSecond(curPtr, start, end, ptrType, state);
    } else if ((opcode >= OS_UNWEIND_CFA_DEF_CFA) && (opcode <= OS_UNWEIND_CFA_GNU_NEGATIVE_OFFSET_EXTENDED)) {
        /* 3、操作码low 6Bits在[0xC-0x2F]的 */
        result = (U64)OsUnwindProcessCfiOpcodeThird(curPtr, end, state);
    } else {
        result = 0;
    }
    return result;
}

/*
* 描述：解析CIE和FDE,解析结果放到state数据结构中
*/
OS_SEC_L2_TEXT U64 OsUnwindProcessCfi(U8 *start, U8 *end, uintptr_t targetLoc,
                                   U32 ptrType, struct OsUnwindState *state)
{
    union OsUnwindPtrType curPtr;
    U64 result = 1ULL;
    uintptr_t value;

    /* 先递归的解析CIE中的cfa指令 */
    if (start != state->cieStart) {
       state->loc = state->org;
       result = OsUnwindProcessCfi(state->cieStart, state->cieEnd, 0, ptrType, state);
       if ((targetLoc == 0) && (state->label == NULL)) {
            return result;
       }
    }

    /* 再解析FDE中的cfa指令 */
    for(curPtr.u8Ptr = start; ((result != 0) && (curPtr.u8Ptr < end));) {
        switch (*curPtr.u8Ptr >> OS_UNWEIND_PROCESSS_CFA_OPCODE_BITS) {
            case OS_UNWEIND_PROCESSS_CFA_NEED_PARSED:
                result = (U64)OsUnwindProcessCfiOpcode(&curPtr, start, end, ptrType, state);
                if ((result & OS_UNWEIND_PROCESSS_DIRECT_RETURN) != 0) {
                    result &= (~OS_UNWEIND_PROCESSS_DIRECT_RETURN);
                    return result;
                }
                break;
            case OS_UNWEIND_PROCESSS_CFA_ADV:
                result = (U64)OsUnwindAdvanceLoc(*curPtr.u8Ptr++ & OS_UNWEIND_PROCESSS_CFA_MASK, state);
                break;
            case OS_UNWEIND_PROCESSS_CFA_OFFSET:
                value = *curPtr.u8Ptr++ & OS_UNWEIND_PROCESSS_CFA_MASK;
                OsUnwindSetRule(value, OS_UNWEIND_MEMORY, OsUnwindGetUleb(&curPtr.u8Ptr, end), state);
                break;
            case OS_UNWEIND_PROCESSS_CFA_RESTORE:
                OsUnwindSetRule(*curPtr.u8Ptr++ & OS_UNWEIND_PROCESSS_CFA_MASK, OS_UNWEIND_NOWHERE, 0, state);
                break;
            default:
                break;    
        }

        if (curPtr.u8Ptr > end) {
            result = 0;
        }

        if ((result != 0) && (targetLoc != 0) && (targetLoc < state->loc)) {
            return 1UL;
        }
    }

    return (U64)((result != 0) && (curPtr.u8Ptr == end) && (targetLoc == 0 || (state->label == NULL)));
}

/*
* 描述：根据hdr获得TableSize
*/
OS_SEC_L2_TEXT U32 OsUnwindGetTableSizeByHdr(U8 *hdr)
{
    U32 tableSize = 0;
    if (hdr == NULL) {
        return tableSize;
    }

    switch (((U32)(hdr[OS_UNWEIND_TABLENC_OFFSET])) & OS_UNWEIND_EH_PE_FORM) {
        case OS_UNWEIND_EH_PE_NATIVE:
            tableSize = sizeof(uintptr_t);
            break;
        case OS_UNWEIND_EH_PE_DATA2:
            tableSize = sizeof(U16);
            break;
        case OS_UNWEIND_EH_PE_DATA4:
            tableSize = OS_BYTES_PER_WORD;
            break;
        case OS_UNWEIND_EH_PE_DATA8:
            tableSize = OS_BYTES_PER_DWORD;
            break;
        default:
            tableSize = 0;
            break;
    }
    return tableSize;
}
/*
* 描述：根据pc获得Fde
*/
OS_SEC_L2_TEXT static uintptr_t OsUnwindGetFdeByPcInner(U8 *endPtr, U8 **curPtr, uintptr_t pc, U8 *hdr,
                                                     struct OsUnwindLocation *loc)
{
    uintptr_t fdeAddr = 0;
    U32 tableSize;
    U32 fdeCount;
    U8 *current = NULL;

    tableSize = OsUnwindGetTableSizeByHdr(hdr);
    fdeCount = (U32)OsUnwindReadPointer(curPtr, endPtr, hdr[OS_UNWEIND_FDECOUNT_OFFSET]);
    /* 计算fdeCount存储到i变量，并进行合法性判断 */
    if ((fdeCount > 0) && ((((U32)(endPtr - *curPtr)) / (tableSize << 1)) == fdeCount) &&
        ((((U32)(endPtr - *curPtr)) % (tableSize << 1)) == 0)) {
        do {
            current = *curPtr + ((uintptr_t)fdeCount >> 1) *((uintptr_t)tableSize << 1);
            loc->startLoc = OsUnwindReadPointer(&current, current + tableSize, hdr[OS_UNWEIND_TABLENC_OFFSET]);
            if (pc < loc->startLoc) {
                fdeCount = (fdeCount >> 1);
            } else {
                *curPtr = current - tableSize;
                fdeCount = ((fdeCount + 1) >> 1);
            }
        } while ((loc->startLoc != 0) && (fdeCount > 1));
        if (fdeCount == 1) {
            loc->startLoc = OsUnwindReadPointer(curPtr, *curPtr + tableSize, hdr[OS_UNWEIND_TABLENC_OFFSET]);
            if ((loc->startLoc != 0) && (pc >= loc->startLoc)) {
                fdeAddr = OsUnwindReadPointer(curPtr, *curPtr + tableSize, hdr[OS_UNWEIND_TABLENC_OFFSET]);
            }
        }
    }
    return fdeAddr;
}

/*
* 描述：OsUnwindGetFdeByPc 根据pc获得Fde
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindGetFdeByPc(uintptr_t pc, U8 **curPtr, struct OsUnwindLocation *loc)
{
    U32 tableSize;
    U8 *hdr = NULL;
    U8 *endPtr = NULL;
    uintptr_t fdeAddr = 0;
    struct OsUnwindTable *table;

    table = (struct OsUnwindTable *)OsUnwindFindTableByPc(pc);
    if ((table == NULL) || OS_NOT_ALIGN_CHK(table->size, sizeof(U32))) {
        return fdeAddr;
    }

    hdr = table->header;
    if ((hdr == NULL) || (hdr[OS_UNWEIND_VERSION_OFFSET] != OS_UNWEIND_DEFAULT_VERSION)) {
        return fdeAddr;
    }

    tableSize = OsUnwindGetTableSizeByHdr(hdr);
    *curPtr = hdr + sizeof(uintptr_t);
    endPtr = hdr + table->hdrsz;
    loc->startLoc = 0;
    loc->endLoc = 0;
    if ((tableSize != 0) &&
        (OsUnwindReadPointer(curPtr, endPtr, hdr[OS_UNWEIND_FRAMENC_OFFSET]) == table->address)) {
        fdeAddr = OsUnwindGetFdeByPcInner(endPtr, curPtr, pc, hdr, loc);
    }
    return fdeAddr;
}

/*
* 描述：OsUnwindGetFdeByPc 解析出Augmentation String字段
*/
OS_SEC_L2_TEXT void OsUnwindGetAugmentationByCie(U32 **cie, U8 **curPtr, struct OsUnwindFrameInfo *frame)
{
    U8 *end = (U8 *)(*cie + 1) + **cie;
    frame->callFrame = 1;
    /* 解析出Augmentation String字段 */
    if ((*(++(*curPtr))) == 0) {
        (++(*curPtr));
        return;
    }

    if (**curPtr == 'z') {
        while ((++(*curPtr)) < end && ((**curPtr) != 0)) {
            switch (**curPtr) {
                case 'L':
                case 'P':
                case 'R':
                    continue;
                case 'S':
                    frame->callFrame = 0;
                    continue;
                default:
                    break;
            }
            break;
        }
    }
    
    if (*curPtr >= end || **curPtr != 0) {
        *cie = NULL;
    }

    ++(*curPtr);
    return;
}

/*
* 描述：OsUnwindGetStateByCie 根据Cie获得CieEnd标记
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindGetCieEndByCie(U32 **cie, U32 **fde, U8 **curPtr,
                                             struct OsUnwindLocation *loc, struct OsUnwindFrameInfo *frame)
{
    uintptr_t pc = OS_UNWEIND_FRAME_PC(frame) - frame->callFrame;
    U32 ptrType = OS_UNWEIND_POINTERTYPE_INV;
    uintptr_t cieEnd = 0;
    uintptr_t startLoc;
    /* 根据fde中的指针找到对应cie */
    *cie = (U32 *)OsUnwindGetCieByFde(*fde);
    if ((*cie == NULL) || (*cie == &g_unwindBadCie) || (*cie == &g_unwindNotFde)) {
        return cieEnd;
    }

    *curPtr = (U8 *)(*fde + OS_UNWEIND_SKIP_AUG_OFFSET);

    ptrType = OsUnwindFdePointerType(*cie);
    if (ptrType != OS_UNWEIND_POINTERTYPE_INV) {
        startLoc = OsUnwindReadPointer(curPtr, (U8 *)(*fde + 1) + **fde, ptrType);
        if (startLoc == loc->startLoc) {
            if ((ptrType & OS_UNWEIND_EH_PE_INDIRECT) == 0) {
                ptrType &= (OS_UNWEIND_EH_PE_FORM | OS_UNWEIND_EH_PE_SIGNED);
            }
            loc->endLoc = loc->startLoc + OsUnwindReadPointer(curPtr, (U8 *)(*fde + 1) + **fde, ptrType);
            if (pc >= loc->endLoc) {
                *cie = NULL;
            }
        }
    } else {
        *cie = NULL;
    }

    if (*cie == NULL) {
        return cieEnd;
    }

    cieEnd = (uintptr_t)*curPtr;
    *curPtr = (U8 *)(*cie + OS_UNWEIND_SKIP_AUG_OFFSET);
    OsUnwindGetAugmentationByCie(cie, curPtr, frame);
    return cieEnd;

}

/*
* 描述：OsUnwindGetFdeByPc 解析出RetAddr字段
*/
OS_SEC_L2_TEXT uintptr_t OsUnwindGetRetAddr(U32 **cie, U32 **fde, U8 **curPtr,
                                      struct OsUnwindLocation *loc, struct OsUnwindState *state)
{
    uintptr_t augSize;
    uintptr_t retAddrReg = 0;
    U8 *end = (U8 *)(*cie + 1) + **cie;
    /* 获取代码对齐因子 */
    state->codeAlign = OsUnwindGetUleb(curPtr, end);
    /* 获取数据对齐因子 */
    state->dataAlign = (uintptr_t)OsUnwindGetSleb(curPtr, end);
    if ((state->codeAlign == 0) || (state->dataAlign == 0) || (*curPtr >= end)) {
        *cie = NULL;
        return retAddrReg;
    }
    retAddrReg = (uintptr_t)(*((*curPtr)++));

    if (((U8 *)(*cie + OS_UNWEIND_SKIP_AUG_OFFSET))[1] == 'z') {
        augSize = OsUnwindGetUleb(curPtr, end);
        *curPtr +=augSize;
    }
    if ((*curPtr > end) || (retAddrReg >= OS_UNWEIND_ARRAY_SIZE(g_unwindRegTableInfo)) ||
        OS_UNWEIND_FRAME_REG_INVALID(retAddrReg) ||
        (g_unwindRegTableInfo[retAddrReg].width != sizeof(uintptr_t))) {
        *cie = NULL;
    }

    if (*cie == NULL) {
        return retAddrReg;
    }

    state->cieStart = *curPtr;
    *curPtr = state->cieEnd;
    state->cieEnd = end;
    end = (U8 *)(*fde + 1) + **fde;

    if (((char *)(*cie + OS_UNWEIND_SKIP_AUG_OFFSET))[1] == 'z') {
        augSize = OsUnwindGetUleb(curPtr, end);
        if ((*curPtr += augSize) > end) {
            *fde = NULL;
        }
    }

    state->org = loc->startLoc;
    if (memcpy_s(&(state->cfa), sizeof(state->cfa), &g_unwindBadCfa, sizeof(g_unwindBadCfa)) != EOK) {
        OS_GOTO_SYS_ERROR();
    }
    return retAddrReg;
}

/*
* 描述：OsUnwindGetFrameByStateLocFirst 第一轮解析出栈帧详细信息
*/
OS_SEC_L2_TEXT U32 OsUnwindFirstGetFrameByStateLoc(struct OsUnwindFrameInfo *frame, struct OsUnwindLocation *loc,
                                                struct OsUnwindState *state)
{
    U32 i;
    uintptr_t cfa;
    /* 首先根据指令解析结果，计算cfa的值 */
    cfa = OS_UNWEIND_FRAME_REG(frame, state->cfa.reg, uintptr_t) +state->cfa.offs;
    loc->startLoc = MIN(OS_UNWEIND_FRAME_SP(frame), cfa);
    loc->endLoc = MAX(OS_UNWEIND_FRAME_SP(frame), cfa);

    for (i = 0; i < OS_UNWEIND_ARRAY_SIZE(state->regs); ++i) {
        if (OS_UNWEIND_FRAME_REG_INVALID(i)) {
            if (state->regs[i].where == OS_UNWEIND_NOWHERE) {
                continue;
            }
            return OS_FAIL;
        }
    }
    return OS_OK;
}

/*
* 描述：栈帧寄存器赋值
*/
OS_SEC_L2_TEXT U32 OsUnwindSetFrameRegValue(struct OsUnwindFrameInfo *frame, uintptr_t width,
                                         U32 pos, uintptr_t valueAddr)
{
    switch (width) {
        case sizeof(U8):
            OS_UNWEIND_FRAME_REG(frame, pos, U8) = *(U8 *)valueAddr;
            break;
        case sizeof(U16):
            OS_UNWEIND_FRAME_REG(frame, pos, U16) = *(U16 *)valueAddr;
            break;
        case sizeof(U32):
            OS_UNWEIND_FRAME_REG(frame, pos, U32) = *(U32 *)valueAddr;
            break;
        case sizeof(U64):
            OS_UNWEIND_FRAME_REG(frame, pos, U64) = *(U64 *)valueAddr;
            break;
        default:
            return OS_FAIL;
    }
    return OS_OK;
}

/*
* 描述：OsUnwindGetFrameByStateLocSecond 第二轮解析出栈帧详细信息
*/
OS_SEC_L2_TEXT U32 OsUnwindSecondGetFrameByStateLoc(struct OsUnwindFrameInfo *frame, struct OsUnwindLocation *loc,
                                                struct OsUnwindState *state)
{
    U32 i;
    uintptr_t cfa;
    uintptr_t *fptr;
    uintptr_t addr;
    uintptr_t valueAddr;

    /* 首先根据指令解析结果，计算cfa的值 */
    cfa = OS_UNWEIND_FRAME_REG(frame, state->cfa.reg, uintptr_t) + state->cfa.offs;
    fptr = (uintptr_t *)(&frame->regs);
    /* 第2遍轮询解析 */
    for (i = 0; i < OS_UNWEIND_ARRAY_SIZE(state->regs); ++i, fptr++) {
        if (OS_UNWEIND_FRAME_REG_INVALID(i)) {
            continue;
        }
        switch (state->regs[i].where) {
            case OS_UNWEIND_NOWHERE:
                if ((g_unwindRegTableInfo[i].width != sizeof(OS_UNWEIND_FRAME_SP(frame))) || 
                    ((uintptr_t)(&OS_UNWEIND_FRAME_REG(frame, i, uintptr_t)) != (uintptr_t)(&OS_UNWEIND_FRAME_SP(frame)))) {
                        continue;
                }
                OS_UNWEIND_FRAME_SP(frame) = cfa;
                break;

            case OS_UNWEIND_REGISTER:
                valueAddr = (uintptr_t)(&(state->regs[i].value));
                if (OsUnwindSetFrameRegValue(frame, g_unwindRegTableInfo[i].width, i, valueAddr) != OS_OK) {
                    return OS_FAIL;
                }
                break;
            case OS_UNWEIND_VALUE:
                if (g_unwindRegTableInfo[i].width != sizeof(uintptr_t)) {
                    return OS_FAIL;
                }
                OS_UNWEIND_FRAME_REG(frame, i, uintptr_t) = cfa + state->regs[i].value * state->dataAlign;
                break;
            case OS_UNWEIND_MEMORY:
                addr = cfa + state->regs[i].value * state->dataAlign;
                if (((state->regs[i].value * state->dataAlign) % sizeof(uintptr_t)) || (addr < loc->startLoc) ||
                    (addr + sizeof(uintptr_t) < addr) || (addr + sizeof(uintptr_t) > loc->endLoc)) {
                    return OS_FAIL;
                }
                if (OsUnwindSetFrameRegValue(frame, g_unwindRegTableInfo[i].width, i , addr) != OS_OK) {
                    return OS_FAIL;
                }
                break;
            default:
                return OS_FAIL;
        }
    }
    return OS_OK;
}

/*
* 描述：OsUnwindGetFrameByStateLoc 解析出栈帧详细信息
*/
OS_SEC_L2_TEXT U32 OsUnwindGetFrameByStateLoc(struct OsUnwindFrameInfo *frame, struct OsUnwindLocation *loc,
                                           struct OsUnwindState *state)
{
    U32 ret;
    /* 获取第一轮UnwindState解析结果 */
    ret = OsUnwindFirstGetFrameByStateLoc(frame, loc, state);
    if (ret !=OS_OK) {
        return ret;
    }

    /* 获取第二轮UnwindState解析结果 */
    ret = OsUnwindSecondGetFrameByStateLoc(frame, loc, state);
    return ret;
}

/*
* 描述：根据当前帧pc，寻找当前栈帧
*/
OS_SEC_L2_TEXT U32 OsUnwindPerFrame(struct OsUnwindFrameInfo *frame)
{
    U32 *fde = NULL;
    U8 *curPtr = NULL;
    U8 *end = NULL;
    U32 *cie = NULL;
    uintptr_t pc = OS_UNWEIND_FRAME_PC(frame) - frame->callFrame;
    struct OsUnwindLocation loc = {0, 0};
    U32 ptrType = OS_UNWEIND_POINTERTYPE_INV;
    uintptr_t retAddrReg;
    struct OsUnwindState state;

    /* 判断LR PC是否落入TEXT段 */
    if (!OsUnwindPcLrInIsTextSec(frame)) {
        return OS_FAIL;
    }

    /* 根据PC计算FDE */
    fde = (U32 *)OsUnwindGetFdeByPc(pc, &curPtr, &loc);
    if (fde == NULL) {
        return OS_FAIL;
    }

    /* 根据FDE计算CIE */
    cie = (U32 *)OsUnwindGetCieByFde(fde);
    if (cie == NULL) {
        return OS_FAIL;
    }

    /* 解析出CIE中的length、CIE ID、Version、Augmentation String字段 */
    if (memset_s(&state, sizeof(struct OsUnwindState), 0 , sizeof(struct OsUnwindState)) != EOK) {
        OS_GOTO_SYS_ERROR();
    }
    state.cieEnd = (U8 *)OsUnwindGetCieEndByCie(&cie, &fde, &curPtr, &loc, frame);
    if ((cie == NULL) || (fde == NULL)) {
        return OS_FAIL;
    }

    /* 解析出CIE中的code alignment factor、data alignment factor、return address register、 */
    /* augmentation data length、augmentation data字段*/
    retAddrReg = OsUnwindGetRetAddr(&cie, &fde, &curPtr, &loc, &state);
    if ((cie == NULL) || (fde == NULL)) {
        return OS_FAIL;
    }

    /* 解析出CIE的initial instruction中的cfa指令和FDE的call frame instruction中的cfa指令，解析结果放到state数据结构中 */
    end = (U8 *)(fde + 1) + *fde;
    if ((OsUnwindProcessCfi(curPtr, end, pc, ptrType, &state) == 0) || (state.loc > loc.endLoc) ||
        (state.regs[retAddrReg].where == OS_UNWEIND_NOWHERE) || (state.cfa.reg >= OS_UNWEIND_ARRAY_SIZE(g_unwindRegTableInfo)) ||
        (g_unwindRegTableInfo[state.cfa.reg].width != sizeof(uintptr_t)) || ((state.cfa.offs % sizeof(uintptr_t) != 0))) {
        return OS_UNWEIND_PROCESSS_CFI_FAIL;
    }

    /* 更新帧信息 */
    if ((frame->callFrame != 0) && (OsUnwindIsDefaultRa(state.regs[retAddrReg], state.dataAlign) == 0)) {
        frame->callFrame = 0;
    }

    /* 根据指令解析结果更新寄存器集的值 */
    return OsUnwindGetFrameByStateLoc(frame, &loc, &state);
}

/*
* 描述：OsUnwindGetFirstFrame 获取调用栈第一帧
*/
OS_SEC_ALW_INLINE INLINE U32 OsUnwindGetFirstFrame(const struct TagTskCb *task, 
                                                struct OsUnwindFrameInfo *frameInfo,
                                                U32 *index, uintptr_t *list)
{
    struct TagHwContext *contex = NULL;
    struct ExcInfo *excInfo = NULL;
    bool excFlag = (CUR_NEST_COUNT > 0) ? TRUE : FALSE;
    frameInfo->callFrame = 0;
    if (excFlag) {
        excInfo = OS_EXC_INFO_ADDR;
        list[*index] = excInfo->regInfo.elr;
        *index = *index + 1;
        /* 硬件平台保存的任务上下文再异常sp后再压栈TskContext大小处 */
        contex = (struct TagHwContext *)(excInfo->sp - sizeof(struct TskContext));
        frameInfo->regs.sp = excInfo->sp;
        frameInfo->regs.pc = contex->pc;
        frameInfo->regs.lr = contex->lr;
    } else {
        if ((task == NULL) || (task == RUNNING_TASK)) {
            frameInfo->regs.lr = OsGetLR();
            frameInfo->regs.sp = OsGetSp();
            frameInfo->regs.pc = OsGetPC();
        } else {
            if ((task->taskStatus & OS_TSK_RUNNING) != 0) {
                return OS_FAIL;
            }
            frameInfo->regs.sp = (uintptr_t)(task->stackPointer) + sizeof(struct TskContext);
            frameInfo->regs.pc = ((struct TskContext *)(task->stackPointer))->elr;
            frameInfo->regs.lr = ((struct TskContext *)(task->stackPointer))->x30;
        }
    }
    return OS_OK;
}

/*
* 描述：检查解析后的数据帧是否正确
*/
OS_SEC_L2_TEXT bool OsUnwindCheckFrameValid(struct OsUnwindFrameInfo *frameInfo)
{
    bool isInText = OsUnwindPcLrInIsTextSec(frameInfo);
    return isInText && (OS_UNWEIND_FRAME_PC(frameInfo) != OS_UNWEIND_FRAME_LR(frameInfo));
}

/*
* 描述：Unwind DW2协议标准解析eh_frame段获取调用栈信息
*/
OS_SEC_L2_TEXT void OsUnwindGetStackTrace(const struct TagTskCb *task, U32 *maxDepth, uintptr_t *list)
{
    U32 ret;
    U32 index = 0;
    uintptr_t intSave;
    struct OsUnwindFrameInfo frameInfo = {0};

    /* 加锁期间完成对共享全局资源初始化+当前栈帧获取 */
    intSave = OsUnwindSplIrqLock();
    if (!g_unwindInitFlag) {
        ret = OsUnwindInit();
        if (ret != OS_OK) {
            *maxDepth = index;
            OsUnwindSplIrqUnLock(intSave);
            return;
        }
        g_unwindInitFlag = TRUE;
    }

    /* 获取调用栈第一帧 */
    if (OsUnwindGetFirstFrame(task, &frameInfo, &index, list) != OS_OK) {
        *maxDepth = index;
        OsUnwindSplIrqUnLock(intSave);
        return;
    }

    OsUnwindSplIrqUnLock(intSave);

    while (1) {
        /* 根据当前栈帧获取前一个栈帧 */
        ret = OsUnwindPerFrame(&frameInfo);

        if (((ret != OS_OK) && (ret != OS_UNWEIND_PROCESSS_CFI_FAIL)) ||
            (!OsUnwindCheckFrameValid(&frameInfo))) {
            *maxDepth = index;
            break;
        }
        /* 下一帧的PC即当前帧的LR */
        OS_UNWEIND_FRAME_PC(&frameInfo) = OS_UNWEIND_FRAME_LR(&frameInfo);
        list[index++] = OS_UNWEIND_FRAME_PC(&frameInfo);
        if (index >= *maxDepth) {
            break;
        }
    }
    return;
}