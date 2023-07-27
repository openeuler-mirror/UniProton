#ifndef _PRT_IDT_H_
#define _PRT_IDT_H_

#define TRAP_GATE 0xf
#define INTR_GATE 0xe
#define KERNEL_CS 0x8
#define VECTOR_MAX_COUNT 256
#define HWI_RESERVE_MAX  32

#define HWI_DE  0  /* divide error */
#define HWI_DB  1  /* debug exception */
#define HWI_NMI 2  /* nmi interrupt */
#define HWI_BP  3  /* breakpoint */
#define HWI_OF  4  /* overflow */
#define HWI_BR  5  /* bound range exceeded */
#define HWI_UD  6  /* invalid opcode(undefined opcode) */
#define HWI_NM  7  /* device not available(no math coprocessor) */
#define HWI_DF  8  /* double fault */
#define HWI_CSO 9  /* coprocessor segment overrun(reserved) */
#define HWI_TS  10 /* invalid tss */
#define HWI_NP  11 /* segment not present */
#define HWI_SS  12 /* stack-segment fault */
#define HWI_GP  13 /* general protection */
#define HWI_PF  14 /* page fault */
#define HWI_IR  15 /* intel reserved */
#define HWI_MF  16 /* x87 fpu floating-point error(math fault) */
#define HWI_AC  17 /* alignment check */
#define HWI_MC  18 /* machine check */
#define HWI_XM  19 /* simd floating-point exception */
#define HWI_VE  20 /* virtualization exception */
#define HWI_CP  21 /* control protection exception */
/* 22-31 intel reserved. do not use */

struct IdtEntry {
    U16 lobase;
    U16 selector;
    U8  ist:3;
    U8  reserved:5;
    U8  type:4;
    U8  zero:1;
    U8  dpl:2;
    U8  present:1;
    U16 hibase;
    U32 xhibase;
    U32 reserved1;
};

/* A struct describing a pointer to an array of interrupt handlers. This is 
 * in a format suitable for giving to 'lidt'.
 */
struct IdtPtr {
    U16 limit;
    U64 base;             /* The address of the first GDT entry */
};

extern U64 g_osVectors[VECTOR_MAX_COUNT];

extern void OsIdtIrqEnable(U32 index);
extern void OsIdtIrqDisable(U32 index);

#endif