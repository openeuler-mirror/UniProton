/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2026-06-16
 * Description: CMSIS-RTOS version selection header for UniProton.
 */

#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H

/* Change this value to select the CMSIS-RTOS API version built by UniProton. */
#define CMSIS_OS_VER 2

#if (CMSIS_OS_VER == 1)
#include "1.0/cmsis_os1.h"
#elif (CMSIS_OS_VER == 2)
#include "2.0/cmsis_os2.h"
#else
#error CMSIS Only Support Version 1.0 or 2.0
#endif

#endif
