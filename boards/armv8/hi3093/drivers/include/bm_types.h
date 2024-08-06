/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */
#ifndef __BM_TYPES_H__
#define __BM_TYPES_H__

typedef unsigned long uintptr_t;

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define BM_OK 0
#define BM_FAIL (-1)
#define BM_TIMEOUT (-2)
#define BM_WAIT_FOREVER (0xffffffff)

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef int errno_t;

#ifndef INIT_TEXT
#define INIT_TEXT __attribute__((section(".init.text")))
#endif

#undef BIT
#define BIT(n) (1 << (n))

#define WEAK __attribute__((weak))

#endif /* __TYPES_H__ */
