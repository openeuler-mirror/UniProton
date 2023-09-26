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
 * Description: shell los_printf 适配头文件。
 */
#ifndef _LOS_PRINTF_H
#define _LOS_PRINTF_H

#include "stdarg.h"
#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_printf
 * log print level definition, LOS_EMG_LEVEL is set to 0, it means the log is emergency.
 */
#define LOS_EMG_LEVEL    0

/**
 * @ingroup los_printf
 * log print level definition, LOS_COMMOM_LEVEL is set to 1, it means the log is common.
 */
#define LOS_COMMOM_LEVEL (LOS_EMG_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_ERR_LEVEL is set to 2, it means it is a error log.
 */
#define LOS_ERR_LEVEL    (LOS_COMMOM_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_WARN_LEVEL is set to 3, it means it is a warning log.
 */
#define LOS_WARN_LEVEL   (LOS_ERR_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_INFO_LEVEL is set to 4, it means the log is an information.
 */
#define LOS_INFO_LEVEL   (LOS_WARN_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_DEBUG_LEVEL is set to 5, it means it is a debug log.
 */
#define LOS_DEBUG_LEVEL  (LOS_INFO_LEVEL + 1)

#ifdef LOSCFG_SHELL_LK
#define PRINT_LEVEL      LOS_DEBUG_LEVEL
#else
#define PRINT_LEVEL      LOS_ERR_LEVEL
#endif

#ifndef PRINT_ERR
#if PRINT_LEVEL < LOS_ERR_LEVEL
#define PRINT_ERR(fmt, ...)
#else
#ifdef LOSCFG_SHELL_LK
#define PRINT_ERR(fmt, ...) LOS_LkPrint(LOS_ERR_LEVEL, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define PRINT_ERR(fmt, ...) do {           \
    (shprintf("[ERR] "), shprintf(fmt, ##__VA_ARGS__)); \
} while (0)
#endif
#endif
#endif

#ifndef PRINTK
#if PRINT_LEVEL < LOS_COMMOM_LEVEL
#define PRINTK(fmt, ...)
#else
#ifdef LOSCFG_SHELL_LK
#define PRINTK(fmt, ...) LOS_LkPrint(LOS_COMMOM_LEVEL, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define PRINTK(fmt, ...) shprintf(fmt, ##__VA_ARGS__)
#endif
#endif
#endif

typedef VOID (*pf_OUTPUT)(const CHAR *fmt, ...);

extern void shprintf(const char *fmt, ...); /* Conflicting types for 'dprintf' */

extern VOID LOS_LkPrint(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_PRINTF_H */