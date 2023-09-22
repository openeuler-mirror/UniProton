/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_UACCESS_H__
#define __LINUX_UACCESS_H__

#include <string.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/compiler_types.h>

unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);

#define __user

static __always_inline __must_check unsigned long
__copy_to_user(void __user *to, const void *from, unsigned long n)
{
    return copy_to_user(to, from, n);
}

static inline __must_check unsigned long
_copy_from_user(void *to, const void __user *from, unsigned long n)
{
    return copy_from_user(to, from, n);
}

/*
 * The architecture should really override this if possible, at least
 * doing a check on the get_fs()
 */
#ifndef __access_ok
static inline int __access_ok(unsigned long addr, unsigned long size)
{
    return 1;
}
#endif

#define access_ok(addr, size) __access_ok((unsigned long)(addr),(size))

#define put_user(x, ptr)                                                                                               \
    ({                                                                                                                 \
        *ptr = x;                                                                                                      \
        0;                                                                                                             \
    })

#define get_user(x, ptr)                                                                                               \
    ({                                                                                                                 \
        x = *ptr;                                                                                                      \
        0;                                                                                                             \
    })

#endif