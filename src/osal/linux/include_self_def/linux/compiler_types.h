/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_COMPILER_TYPE_H
#define _LINUX_COMPILER_TYPE_H

#define barrier()

#define __chk_user_ptr(x) (void)0

#ifdef __CHECKER__
#define __force    __attribute__((force))
#else
#define __force
#endif

#ifdef CONFIG_ENABLE_MUST_CHECK
#define __must_check __attribute__((__warn_unused_result__))
#else
#define __must_check
#endif

#define __always_inline                 inline __attribute__((__always_inline__))

#endif /* _LINUX_COMPILER_TYPE_H */