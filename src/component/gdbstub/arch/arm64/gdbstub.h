/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 *
 * Author: niutao2@huawei.com
 * Create: 2023-09-20
 * Description: arm64 gdb公共宏定义，宏定义移植自linux内核
 */

#ifndef _GDBSTUB_H_
#define _GDBSTUB_H_

#include "prt_typedef.h"

#define _GP_REGS        33
#define _FP_REGS        32
#define _EXTRA_REGS     3
/*
 * general purpose registers size in bytes.
 * pstate is only 4 bytes. subtract 4 bytes
 */
#define GP_REG_BYTES        (_GP_REGS * 8)
#define DBG_MAX_REG_NUM     (_GP_REGS + _FP_REGS + _EXTRA_REGS)
/*
 * Number of bytes required for gdb_regs buffer.
 * _GP_REGS: 8 bytes, _FP_REGS: 16 bytes and _EXTRA_REGS: 4 bytes each
 * GDB fails to connect for size beyond this with error
 * "'g' packet reply is too long"
 */

#define NUMREGBYTES    ((_GP_REGS * 8) + (_FP_REGS * 16) + \
            (_EXTRA_REGS * 4))

/*
 * #imm16 values used for BRK instruction generation
 * 0x400: for dynamic BRK instruction
 * 0x401: for compile time BRK instruction
 */
#define GDB_DYN_DBG_BRK_IMM        0x400
#define GDB_COMPILED_DBG_BRK_IMM   0x401

#define ESR_ELx_BRK64_ISS_COMMENT_MASK 0xFFFF
#define ESR_ELx_EC_SHIFT    (26)
#define ESR_ELx_EC_WIDTH    (6)
#define ESR_ELx_EC_MASK     (0x3FUL << ESR_ELx_EC_SHIFT)
#define ESR_ELx_EC(esr)     (((esr) & ESR_ELx_EC_MASK) >> ESR_ELx_EC_SHIFT)
#define ESR_ELx_EC_WATCHPT_CUR  (0x35)
/*
 * BRK instruction encoding
 * The #imm16 value should be placed at bits[20:5] within BRK ins
 */    
#define AARCH64_BREAK_MON   0xd4200000

/*
 * BRK instruction for provoking a fault on purpose
 * Unlike kgdb, #imm16 value with unallocated handler is used for faulting.
 */    
#define AARCH64_BREAK_FAULT (AARCH64_BREAK_MON | (FAULT_BRK_IMM << 5))

#define AARCH64_BREAK_KGDB_DYN_DBG  \
    (AARCH64_BREAK_MON | (GDB_DYN_DBG_BRK_IMM << 5))

/*
 * Break point instruction encoding
 */    
#define BREAK_INSTR_SIZE        4

/* Low-level stepping controls. */
#define DBG_MDSCR_SS   (1 << 0)
#define DBG_SPSR_SS    (1 << 21)
#define DBG_SPSR_D     (1 << 9)

/* MDSCR_EL1 enabling bits */
#define DBG_MDSCR_KDE (1 << 13)
#define DBG_MDSCR_MDE (1 << 15)
#define DBG_MDSCR_MASK      ~(DBG_MDSCR_KDE | DBG_MDSCR_MDE)

/* need 2 buffer to store one byte after convert to string*/
#define BYTE_TO_STR_LEN 2
/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x...)    #x
#define __stringify(x...)    __stringify_1(x)

/*
 * Unlike read_cpuid, calls to read_sysreg are never expected to be
 * optimized away or replaced with synthetic values.
 */
#define read_sysreg(r) ({                    \
    uint64_t __val;                        \
    __asm__  volatile("mrs %0, " __stringify(r) : "=r" (__val));    \
    __val;                            \
})

/*
 * The "Z" constraint normally means a zero immediate, but when combined with
 * the "%x0" template means XZR.
 */
#define write_sysreg(v, r) do {                    \
    uint64_t __val = (uint64_t)(v);                    \
    __asm__  volatile("msr " __stringify(r) ", %x0"        \
             : : "rZ" (__val));                \
} while (0)

/* Breakpoint */
#define ARM_BREAKPOINT_EXECUTE    0

/* Watchpoints */
#define ARM_BREAKPOINT_LOAD      1
#define ARM_BREAKPOINT_STORE     2
#define AARCH64_ESR_ACCESS_MASK  (1 << 6)

/* Lengths */
#define ARM_BREAKPOINT_LEN_1    0x1
#define ARM_BREAKPOINT_LEN_2    0x3
#define ARM_BREAKPOINT_LEN_3    0x7
#define ARM_BREAKPOINT_LEN_4    0xf
#define ARM_BREAKPOINT_LEN_5    0x1f
#define ARM_BREAKPOINT_LEN_6    0x3f
#define ARM_BREAKPOINT_LEN_7    0x7f
#define ARM_BREAKPOINT_LEN_8    0xff

/* Kernel stepping */
#define ARM_KERNEL_STEP_NONE        0
#define ARM_KERNEL_STEP_ACTIVE      1
#define ARM_KERNEL_STEP_SUSPEND     2

/*
 * Limits.
 * Changing these will require modifications to the register accessors.
 */
#define ARM_MAX_BRP        16
#define ARM_MAX_WRP        16

/* Virtual debug register bases. */
#define AARCH64_DBG_REG_BVR    0
#define AARCH64_DBG_REG_BCR    (AARCH64_DBG_REG_BVR + ARM_MAX_BRP)
#define AARCH64_DBG_REG_WVR    (AARCH64_DBG_REG_BCR + ARM_MAX_BRP)
#define AARCH64_DBG_REG_WCR    (AARCH64_DBG_REG_WVR + ARM_MAX_WRP)

/* Debug register names. */
#define AARCH64_DBG_REG_NAME_BVR    bvr
#define AARCH64_DBG_REG_NAME_BCR    bcr
#define AARCH64_DBG_REG_NAME_WVR    wvr
#define AARCH64_DBG_REG_NAME_WCR    wcr

/* Accessor macros for the debug registers. */
#define AARCH64_DBG_READ(N, REG, VAL) do {  \
    VAL = read_sysreg(dbg##REG##N##_el1);   \
} while (0)

#define AARCH64_DBG_WRITE(N, REG, VAL) do { \
    write_sysreg(VAL, dbg##REG##N##_el1);   \
} while (0)

#define READ_WB_REG_CASE(OFF, N, REG, VAL)  \
    case (OFF + N):                         \
        AARCH64_DBG_READ(N, REG, VAL);      \
        break

#define WRITE_WB_REG_CASE(OFF, N, REG, VAL) \
    case (OFF + N):                         \
        AARCH64_DBG_WRITE(N, REG, VAL);     \
        break

#define GEN_READ_WB_REG_CASES(OFF, REG, VAL)\
    READ_WB_REG_CASE(OFF,  0, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  1, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  2, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  3, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  4, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  5, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  6, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  7, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  8, REG, VAL);    \
    READ_WB_REG_CASE(OFF,  9, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 10, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 11, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 12, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 13, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 14, REG, VAL);    \
    READ_WB_REG_CASE(OFF, 15, REG, VAL)

#define GEN_WRITE_WB_REG_CASES(OFF, REG, VAL)\
    WRITE_WB_REG_CASE(OFF,  0, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  1, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  2, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  3, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  4, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  5, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  6, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  7, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  8, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF,  9, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 10, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 11, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 12, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 13, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 14, REG, VAL);    \
    WRITE_WB_REG_CASE(OFF, 15, REG, VAL)


#define ID_AA64DFR0_WRPS_SHIFT      20
#define ID_AA64DFR0_BRPS_SHIFT      12
#define AA64DFR0_WRPS_VAL(x) ((((x) >> ID_AA64DFR0_WRPS_SHIFT) & 0xf ) + 1)
#define AA64DFR0_BRPS_VAL(x) ((((x) >> ID_AA64DFR0_BRPS_SHIFT) & 0xf ) + 1)

/* Privilege Levels */
#define AARCH64_BREAKPOINT_EL1    1
#define AARCH64_BREAKPOINT_EL0    2

#define DBG_HMC_HYP        (1 << 13)

#define LAZY_STOP          1
/*
 * This struct defines the way the registers are stored on the stack during an
 * exception. Note that sizeof(struct prt_regs) has to be a multiple of 16 (for
 * stack alignment). struct user_prt_regs must form a prefix of struct prt_regs.
 */    
struct PrtRegs {
    uint64_t regs[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
};

struct GdbCtx
{
    void *sp;
    union {
        struct PrtRegs regs;
        U8 rbuf[sizeof(struct PrtRegs)];
    } regs;
    uint64_t far;
    uint8_t ec;
};

typedef struct HwBreakpoint {
    uintptr_t addr;
    uint64_t ctrl;
    uint32_t len;
    uint32_t state;
} HwBreakpoint_t;

typedef struct Watchpoint {
    uintptr_t addr;
    uint64_t ctrl;
    uint32_t len;
    uint32_t state;
    uintptr_t origAddr;
    uint32_t origLen;
} Watchpoint_t;

struct HwBreakpointCtrl {
    uint32_t reserved   : 19,
    len                 : 8,
    type                : 2,
    privilege           : 2,
    enabled             : 1;
};

#endif /* _GDBSTUB_H_ */
