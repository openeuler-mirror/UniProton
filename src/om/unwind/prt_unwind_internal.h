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
 * Description: unwind模块的模块内头文件
 */
#ifndef PRT_UNWIND_INTERNAL_H
#define PRT_UNWIND_INTERNAL_H

#include "prt_task_external.h"
#include "prt_cpu_external.h"
#include "prt_lib_external.h"

/*
* 模块内变量声明
*/
extern U32 __os_text_start;
extern U32 __os_text_end;
extern U32 __os_unwind_table_start;
extern U32 __os_unwind_table_end;
extern struct OsUnwindTable g_unwindBaseTable;
extern struct OsUnwindCfa g_unwindBadCfa;
extern U32 g_unwindBadCie;
extern U32 g_unwindNotFde;

#if defined(OS_OPTION_SMP)
extern volatile uintptr_t g_unwindSplAddr;
#endif

/*
* 模块内宏定义
*/
#define OS_UNWEIND_CFA_NOP                          0x00U
#define OS_UNWEIND_CFA_SET_LOC                      0x01U
#define OS_UNWEIND_CFA_ADVANCE_LOC1                 0x02U
#define OS_UNWEIND_CFA_ADVANCE_LOC2                 0x03U
#define OS_UNWEIND_CFA_ADVANCE_LOC4                 0x04U
#define OS_UNWEIND_CFA_OFFSET_EXTENDED              0x05U
#define OS_UNWEIND_CFA_RESTORE_EXTENDED             0x06U
#define OS_UNWEIND_CFA_UNDEFINED                    0x07U
#define OS_UNWEIND_CFA_SAME_VALUE                   0x08U
#define OS_UNWEIND_CFA_REGISTER                     0x09U
#define OS_UNWEIND_CFA_REMEMBER_STATE               0x0AU
#define OS_UNWEIND_CFA_RESTORE_STATE                0x0BU
#define OS_UNWEIND_CFA_DEF_CFA                      0x0CU
#define OS_UNWEIND_CFA_DEF_CFA_REGISTER             0x0DU
#define OS_UNWEIND_CFA_DEF_CFA_OFFSET               0x0EU
#define OS_UNWEIND_CFA_DEF_CFA_EXPRESSION           0x0FU
#define OS_UNWEIND_CFA_EXPRESSION                   0x10U
#define OS_UNWEIND_CFA_OFFSET_EXTENDED_SF           0x11U
#define OS_UNWEIND_CFA_DEF_CFA_SF                   0x12U
#define OS_UNWEIND_CFA_DEF_CFA_OFFSET_SF            0x13U
#define OS_UNWEIND_CFA_VAL_OFFSET                   0x14U
#define OS_UNWEIND_CFA_VAL_OFFSET_SF                0x15U
#define OS_UNWEIND_CFA_VAL_EXPRESSION               0x16U
#define OS_UNWEIND_CFA_LO_USER                      0x1CU
#define OS_UNWEIND_CFA_GNU_WINDOW_SAVE              0x2DU
#define OS_UNWEIND_CFA_GNU_ARGS_SIZE                0x2EU
#define OS_UNWEIND_CFA_GNU_NEGATIVE_OFFSET_EXTENDED 0x2FU
#define OS_UNWEIND_CFA_HI_USER                      0x3FU

#define OS_UNWEIND_EH_PE_FORM     0x07U
#define OS_UNWEIND_EH_PE_NATIVE   0x00U
#define OS_UNWEIND_EH_PE_LEB128   0x01U
#define OS_UNWEIND_EH_PE_DATA2    0x02U
#define OS_UNWEIND_EH_PE_DATA4    0x03U
#define OS_UNWEIND_EH_PE_DATA8    0x04U
#define OS_UNWEIND_EH_PE_SIGNED   0x08U
#define OS_UNWEIND_EH_PE_ADJUST   0x70U
#define OS_UNWEIND_EH_PE_ABS      0x00U
#define OS_UNWEIND_EH_PE_PCREL    0x10U
#define OS_UNWEIND_EH_PE_TEXTREL  0x20U
#define OS_UNWEIND_EH_PE_DATAREL  0x30U
#define OS_UNWEIND_EH_PE_FUNCREL  0x40U
#define OS_UNWEIND_EH_PE_ALIGNED  0x50U
#define OS_UNWEIND_EH_PE_INDIRECT 0x80U
#define OS_UNWEIND_EH_PE_OMIT     0xFFU
#define OS_UNWEIND_CIE_ID         0x0U

#define OS_UNWEIND_MAX_STACK_DEPTH 0xaU
#define OS_UNWEIND_FED_HEAD_OFFSET 0x2U
#define OS_UNWEIND_CIE_HEAD_OFFSET 0x4U
#define OS_UNWEIND_DEFAULT_VERSION 0x1U
#define OS_UNWEIND_VERSION_OFFSET  0x0U
#define OS_UNWEIND_FRAMENC_OFFSET  0x1U
#define OS_UNWEIND_FDECOUNT_OFFSET 0x2U
#define OS_UNWEIND_TABLENC_OFFSET  0x3U
#define OS_UNWEIND_POINTERTYPE_INV 0xFFFFFFFFU
#define OS_UNWEIND_SKIP_AUG_OFFSET 0x2U /* 解析跳过augmentation需要的步长 */

#define OS_UNWEIND_LIB128_STEP_SIZE 0x7U
#define OS_UNWEIND_LIB128_MAX_VALUE 0x7FU

#define OS_UNWEIND_PROCESSS_CFA_MASK         0x3fU
#define OS_UNWEIND_PROCESSS_CFA_NEED_PARSED  0x0U
#define OS_UNWEIND_PROCESSS_CFA_ADV          0x1U
#define OS_UNWEIND_PROCESSS_CFA_OFFSET       0x2U
#define OS_UNWEIND_PROCESSS_CFA_RESTORE      0x3U
#define OS_UNWEIND_PROCESSS_CFA_OPCODE_BITS  0x6U
#define OS_UNWEIND_PROCESSS_DIRECT_RETURN    (0x1LU << 63)
#define OS_UNWEIND_PROCESSS_TEXT_LABEL_PAIR  0x2U
#define OS_UNWEIND_PROCESSS_CFI_FAIL       0x2U

#define OS_UNWEIND_MEMBER_OFFSET_OF(type, member)   ((uintptr_t) & ((type *)0)->member)
#define OS_UNWEIND_SIZE_OF_MEMBER(type, member)     ((uintptr_t)(sizeof((((type *)0)->member))))
#define OS_UNWEIND_TABLE_INFO_OFFS(offset, size)    (((offset) % (size)) + ((offset) / (size)))

#define OS_UNWEIND_FRAME_REG(frame, r, t)    (((t *)(frame))[g_unwindRegTableInfo[r].offs])
#define OS_UNWEIND_FRAME_REG_INVALID(r)      (g_unwindRegTableInfo[r].width == 0)

#define OS_UNWEIND_FRAME_PC(frame)      ((frame)->regs.pc)
#define OS_UNWEIND_FRAME_SP(frame)      ((frame)->regs.sp)
#define OS_UNWEIND_FRAME_LR(frame)      ((frame)->regs.lr)
#define OS_UNWEIND_FRAME_FP(frame)      ((void)(frame), 0)

#define OS_UNWEIND_ARRAY_SIZE(arr)       (sizeof(arr) / sizeof((arr)[0]))
#define OS_UNWEIND_ARCH_REG_SIZE        (sizeof(struct TagStackTraceContext) / (sizeof(uintptr_t)))

/*
* 模块内结构体定义
*/

struct OsUnwindRegTable {
    uintptr_t offs;
    size_t width;
};

struct OsUnwindFrameInfo {
    struct TagStackTraceContext regs;
    const struct TagTskCb *task;
    uintptr_t callFrame;
};

struct OsUnwindAddrRange {
    uintptr_t pc;       /* 起始地址 */
    uintptr_t range;    /* 范围 */
};

struct OsUnwindTable {
    struct OsUnwindAddrRange core;
    uintptr_t address;
    uintptr_t size;
    U8 *header;
    uintptr_t hdrsz;
    struct OsUnwindTable *link;
};

enum UnwindItemLocation {
    OS_UNWEIND_NOWHERE,
    OS_UNWEIND_MEMORY,
    OS_UNWEIND_REGISTER,
    OS_UNWEIND_VALUE
};

struct OsUnwindItem {
    enum UnwindItemLocation where;
    U32 resv1;
    uintptr_t value;
};

struct OsUnwindCfa {
    uintptr_t reg;
    uintptr_t offs;
};

struct OsUnwindState {
    uintptr_t loc;
    uintptr_t org;
    U8 *cieStart;
    U8 *cieEnd;
    uintptr_t codeAlign;
    uintptr_t dataAlign;
    struct OsUnwindCfa cfa;
    struct OsUnwindItem regs[OS_UNWEIND_ARCH_REG_SIZE];
    U32 stackDepth : 8;
    U32 version : 8;
    U16 resv1;
    U32 resv2;
    U8 *label;
    U8 *stack[OS_UNWEIND_MAX_STACK_DEPTH];
};

struct OsUnwindHdrHeadTableEntry {
    uintptr_t start;
    uintptr_t fde;
};

struct OsUnwindLocation {
    uintptr_t startLoc;
    uintptr_t endLoc;
};

struct OsUnwindHdrHeadBaseInfo {
    U8 version;
    U8 ehFramePtrEnc;
    U8 fdeCountEnc;
    U8 tableEnc;
    U32 resv;
    uintptr_t ehFramePtr;
    uintptr_t fdeCount;
};

struct OsUnwindHdrHead {
    struct OsUnwindHdrHeadBaseInfo headInfo;
    struct OsUnwindHdrHeadTableEntry table[];
};

/* Unwind 指针类型枚举 */
union OsUnwindPtrType {
    U8          *u8Ptr;
    U16         *u16Ptr;
    S16         *s16Ptr;
    U32         *u32Ptr;
    S32         *s32Ptr;
    uintptr_t   *ulPtr;
};

/*
* 模块内函数声明
*/
extern struct OsUnwindRegTable g_unwindRegTableInfo[OS_UNWEIND_ARCH_REG_SIZE];
extern U64 OsUnwindGetUleb(U8 **current, U8 *end);
extern S64 OsUnwindGetSleb(U8 **current, U8 *end);
extern uintptr_t OsUnwindReadPointer(U8 **start, U8 *end, U32 ptrType);
extern U32 OsUnwindFdePointerType(U32 *cie);
extern uintptr_t OsUnwindGetCieByFde(U32 *fde);
extern U32 OsUnwindInit(void);
extern uintptr_t OsUnwindFindTableByPc(uintptr_t pc);
extern U64 OsUnwindProcessCfi(U8 *start, U8 *end, uintptr_t targetLoc, U32 ptrType, struct OsUnwindState *state);
extern bool OsUnwindPcLrInIsTextSec(struct OsUnwindFrameInfo *frameInfo);

/*
* 描述：模块内内联函数声明
*/
OS_SEC_ALW_INLINE INLINE U8 OsUnwindGetUnaligned8(U8 *numAddr)
{
    return (U8)numAddr[0];
}

OS_SEC_ALW_INLINE INLINE U16 OsUnwindGetUnaligned16(U8 *numAddr)
{
    return (U16)((OsUnwindGetUnaligned8(numAddr + sizeof(U8)) << OS_BITS_PER_BYTE) | OsUnwindGetUnaligned8(numAddr));
}

OS_SEC_ALW_INLINE INLINE U32 OsUnwindGetUnaligned32(U8 *numAddr)
{
    return (U32)((OsUnwindGetUnaligned16(numAddr + sizeof(U16)) << OS_HALF_WORD_BIT_NUM) | OsUnwindGetUnaligned16(numAddr));
}

OS_SEC_ALW_INLINE INLINE U64 OsUnwindGetUnaligned64(U8 *numAddr)
{
    return ((((U64)(OsUnwindGetUnaligned32(numAddr + sizeof(U32)))) << OS_WORD_BIT_NUM) | OsUnwindGetUnaligned32(numAddr));
}

OS_SEC_ALW_INLINE INLINE bool OsUnwindIsDefaultRa(struct OsUnwindItem regs, uintptr_t dataAlign)
{
    return ((regs.where == OS_UNWEIND_MEMORY) && ((regs.value * (dataAlign) + OS_BYTES_PER_WORD) == 0));
}

#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE uintptr_t OsUnwindSplIrqLock(void)
{
    uintptr_t intSave;
    intSave = OsIntLock();
    OsSplLock(&g_unwindSplAddr);
    return intSave;
}
OS_SEC_ALW_INLINE INLINE void OsUnwindSplIrqUnLock(uintptr_t intSave)
{
    OsSplUnlock(&g_unwindSplAddr);
    OsIntRestore(intSave);
}
#else
OS_SEC_ALW_INLINE INLINE uintptr_t OsUnwindSplIrqLock(void)
{
    uintptr_t intSave;
    intSave = OsIntLock();
    return intSave;
}
OS_SEC_ALW_INLINE INLINE void OsUnwindSplIrqUnLock(uintptr_t intSave)
{
    OsIntRestore(intSave);
}
#endif
#endif /* PRT_UNWIND_INTERNAL_H */
