/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_GENERIC_RWONCE_H
#define __ASM_GENERIC_RWONCE_H

/*
 * __unqual_scalar_typeof(x) - Declare an unqualified scalar type, leaving
 *                   non-scalar types unchanged.
 */
/*
 * Prefer C11 _Generic for better compile-times and simpler code. Note: 'char'
 * is not type-compatible with 'signed char', and we define a separate case.
 */
#define __scalar_type_to_expr_cases(type)                \
        unsigned type:    (unsigned type)0,            \
        signed type:    (signed type)0

#define __unqual_scalar_typeof(x) __typeof__(                \
        _Generic((x),                        \
             char:    (char)0,                \
             __scalar_type_to_expr_cases(char),        \
             __scalar_type_to_expr_cases(short),        \
             __scalar_type_to_expr_cases(int),        \
             __scalar_type_to_expr_cases(long),        \
             __scalar_type_to_expr_cases(long long),    \
             default: (x)))

/*
 * Use __READ_ONCE() instead of READ_ONCE() if you do not require any
 * atomicity. Note that this may result in tears!
 */
#ifndef READ_ONCE
#define READ_ONCE(x)    (*(const volatile __unqual_scalar_typeof(x) *)&(x))
#endif

#define WRITE_ONCE(x, val)                        \
do {                                    \
    *(volatile __typeof__(x) *)&(x) = (val);                \
} while (0)

#endif    /* __ASM_GENERIC_RWONCE_H */