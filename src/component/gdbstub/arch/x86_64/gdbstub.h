#ifndef _GDBSTUB_H_
#define _GDBSTUB_H_

#include "prt_typedef.h"

#define BREAK_INSTR_SIZE 1
/* 17 64 bit regs and 5 32 bit regs */
#define NUMREGBYTES     ((17 * 8) + (7 * 4))
#define DBG_MAX_REG_NUM         24

#define BREAK_INST 0xcc

enum RegNames {
    GDB_AX,         /* 0 */
    GDB_BX,         /* 1 */
    GDB_CX,         /* 2 */
    GDB_DX,         /* 3 */
    GDB_SI,         /* 4 */
    GDB_DI,         /* 5 */
    GDB_BP,         /* 6 */
    GDB_SP,         /* 7 */
    GDB_R8,         /* 8 */
    GDB_R9,         /* 9 */
    GDB_R10,        /* 10 */
    GDB_R11,        /* 11 */
    GDB_R12,        /* 12 */
    GDB_R13,        /* 13 */
    GDB_R14,        /* 14 */
    GDB_R15,        /* 15 */
    GDB_PC,         /* 16 */
    GDB_PS,         /* 17 */
    GDB_CS,         /* 18 */
    GDB_SS,         /* 19 */
    GDB_DS,         /* 20 */
    GDB_ES,         /* 21 */
    GDB_FS,         /* 22 */
    GDB_GS,         /* 23 */
    GDB_ORIG_AX = 0x39,
};

struct PrtRegs {
    U64 ax;
    U64 bx;
    U64 cx;
    U64 dx;
    U64 si;
    U64 di;
    U64 bp;
    U64 r8;
    U64 r9;
    U64 r10;
    U64 r11;
    U64 r12;
    U64 r13;
    U64 r14;
    U64 r15;
    U64 unused0;
    U64 unused1;
    U64 unused2;
    U64 unused3;
    U64 ip;
    U64 cs;
    U64 flags;
    U64 sp;
    U64 ss;
};

struct GdbCtx
{
    void *sp;
    union {
        struct PrtRegs regs;
        U8 rbuf[sizeof(struct PrtRegs)];
    } regs;
};
#endif /* _GDBSTUB_H_ */
