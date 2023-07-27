#ifndef _PRT_VECTOR_H_
#define _PRT_VECTOR_H_

#include "securec.h"
#include "prt_typedef.h"

struct StackFrame {
    U64 rax;
    U64 rbx;
    U64 rcx;
    U64 rdx;
    U64 rsi;
    U64 rdi;
    U64 rbp;
    U64 r8;
    U64 r9;
    U64 r10;
    U64 r11;
    U64 r12;
    U64 r13;
    U64 r14;
    U64 r15;
    U64 intNumber;
    U64 rbpFrame;
    U64 ripFrame;
    U64 error;
    U64 rip;
    U64 cs;
    U64 rFlags;
    U64 rsp;
    U64 ss;
};

#endif