/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H
#include "prt_typedef.h"
#include "sys/types.h"
#include <string.h>

typedef S8  s8;
typedef U8  u8;
typedef S16 s16;
typedef U16 u16;
typedef S32 s32;
typedef U32 u32;
typedef S64 s64;
typedef U64 u64;

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__

typedef unsigned int __bitwise gfp_t;

struct list_head {
    struct list_head *next, *prev;
};

#endif /* _LINUX_TYPES_H */
