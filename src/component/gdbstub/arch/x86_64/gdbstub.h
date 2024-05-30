#ifndef _GDBSTUB_H_
#define _GDBSTUB_H_

#include "prt_typedef.h"

#define BREAK_INSTR_SIZE 1
/* 17 64 bit regs and 5 32 bit regs */
#define NUMREGBYTES     ((17 * 8) + (7 * 4))
#define DBG_MAX_REG_NUM         24

#define BREAK_INST 0xcc

/* Indicate the register numbers for a number of the specific
   debug registers.  Registers 0-3 contain the addresses we wish to trap on */
#define DR_FIRSTADDR 0        /* u_debugreg[DR_FIRSTADDR] */
#define DR_LASTADDR 3         /* u_debugreg[DR_LASTADDR]  */

#define DR_STATUS 6           /* u_debugreg[DR_STATUS]     */
#define DR_CONTROL 7          /* u_debugreg[DR_CONTROL] */

/* Define a few things for the status register.  We can use this to determine
   which debugging register was responsible for the trap.  The other bits
   are either reserved or not of interest to us. */

/* Define reserved bits in DR6 which are always set to 1 */
#define DR6_RESERVED	(0xFFFF0FF0)

#define DR_TRAP0	(0x1)		/* db0 */
#define DR_TRAP1	(0x2)		/* db1 */
#define DR_TRAP2	(0x4)		/* db2 */
#define DR_TRAP3	(0x8)		/* db3 */
#define DR_TRAP_BITS	(DR_TRAP0|DR_TRAP1|DR_TRAP2|DR_TRAP3)

#define DR_STEP		(0x4000)	/* single-step */
#define DR_SWITCH	(0x8000)	/* task switch */

/* Now define a bunch of things for manipulating the control register.
   The top two bytes of the control register consist of 4 fields of 4
   bits - each field corresponds to one of the four debug registers,
   and indicates what types of access we trap on, and how large the data
   field is that we are looking at */

#define DR_CONTROL_SHIFT 16 /* Skip this many bits in ctl register */
#define DR_CONTROL_SIZE 4   /* 4 control bits per register */

#define DR_RW_EXECUTE (0x0)   /* Settings for the access types to trap on */
#define DR_RW_WRITE (0x1)
#define DR_RW_READ (0x3)

#define DR_LEN_1 (0x0) /* Settings for data length to trap on */
#define DR_LEN_2 (0x4)
#define DR_LEN_4 (0xC)
#define DR_LEN_8 (0x8)

/* The low byte to the control register determine which registers are
   enabled.  There are 4 fields of two bits.  One bit is "local", meaning
   that the processor will reset the bit after a task switch and the other
   is global meaning that we have to explicitly reset the bit.  With linux,
   you can use either one, since we explicitly zero the register when we enter
   kernel mode. */

#define DR_LOCAL_ENABLE_SHIFT 0    /* Extra shift to the local enable bit */
#define DR_GLOBAL_ENABLE_SHIFT 1   /* Extra shift to the global enable bit */
#define DR_LOCAL_ENABLE (0x1)      /* Local enable for reg 0 */
#define DR_GLOBAL_ENABLE (0x2)     /* Global enable for reg 0 */
#define DR_ENABLE_SIZE 2           /* 2 enable bits per register */

#define DR_LOCAL_ENABLE_MASK (0x55)  /* Set  local bits for all 4 regs */
#define DR_GLOBAL_ENABLE_MASK (0xAA) /* Set global bits for all 4 regs */

/* The second byte to the control register has a few special things.
   We can slow the instruction pipeline for instructions coming via the
   gdt or the ldt if we want to.  I am not sure why this is an advantage */

#ifdef __i386__
#define DR_CONTROL_RESERVED (0xFC00) /* Reserved by Intel */
#else
#define DR_CONTROL_RESERVED (0xFFFFFFFF0000FC00UL) /* Reserved */
#endif

#define DR_LOCAL_SLOWDOWN (0x100)   /* Local slow the pipeline */
#define DR_GLOBAL_SLOWDOWN (0x200)  /* Global slow the pipeline */

/*
 * The compiler should not reorder volatile asm statements with respect to each
 * other: they should execute in program order. However GCC 4.9.x and 5.x have
 * a bug (which was fixed in 8.1, 7.3 and 6.5) where they might reorder
 * volatile asm. The write functions are not affected since they have memory
 * clobbers preventing reordering. To prevent reads from being reordered with
 * respect to writes, use a dummy memory operand.
 */

#define __FORCE_ORDER "m"(*(unsigned int *)0x1000UL)

/* Available HW breakpoint length encodings */
#define X86_BREAKPOINT_LEN_X        0x40
#define X86_BREAKPOINT_LEN_1        0x40
#define X86_BREAKPOINT_LEN_2        0x44
#define X86_BREAKPOINT_LEN_4        0x4c
#define X86_BREAKPOINT_LEN_8        0x48

/* Available HW breakpoint type encodings */

/* trigger on memory write */
#define X86_BREAKPOINT_WRITE    0x81
/* trigger on memory read or write */
#define X86_BREAKPOINT_RW        0x83

/* debug address registers */
#define DR0    0
#define DR1    1
#define DR2    2
#define DR3    3

/* debug statuc register */
#define DR6    6

/* debug control register */
#define DR7    7

/* Total number of available HW breakpoint registers */
#define HBP_NUM 4

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

struct HwBrkInfo {
    uintptr_t    addr;
    int          len;
    uint8_t      enabled;
    uint8_t      type;
};
#endif /* _GDBSTUB_H_ */
