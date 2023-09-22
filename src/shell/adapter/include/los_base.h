/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-25
 * Description: shell los_base 适配头文件。
 */
#ifndef _LOS_BASE_H
#define _LOS_BASE_H

#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOS_OK
#define LOS_OK  OS_OK
#endif

#ifndef LOS_NOK
#define LOS_NOK OS_FAIL
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef STATIC
#define STATIC  static
#endif

#ifndef FALSE
#define FALSE   0U
#endif

#ifndef TRUE
#define TRUE    1U
#endif

#ifndef LOS_MOD_SHELL
#define LOS_MOD_SHELL OS_MID_SHELL
#endif

#ifndef LOS_ERRNO_OS_ERROR
#define LOS_ERRNO_OS_ERROR OS_ERRNO_BUILD_ERROR  
#endif

#if (OS_HARDWARE_PLATFORM == OS_CORTEX_M4)
#define LOSCFG_ARCH_ARM_CORTEX_M
#endif

#ifndef LITE_OS_SEC_TEXT_MINOR
#define LITE_OS_SEC_TEXT_MINOR  /* __attribute__((section(".text.ddr"))) */
#endif

typedef size_t  BOOL;
typedef U8      UINT8;
typedef U32     UINT32;
typedef S32     INT32;
typedef char    CHAR;
typedef uintptr_t UINTPTR;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_BASE_H */