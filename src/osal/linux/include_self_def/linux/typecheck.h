/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TYPECHECK_H
#define _LINUX_TYPECHECK_H

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({    type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    1; \
})

#endif /* _LINUX_TYPECHECK_H */