/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-24
 * Description: 网络
 */
#ifndef __CC_H__
#define __CC_H__

#include "cpu.h"

#define U16_F "4d"
#define S16_F "4d"
#define X16_F "4X"
#define U32_F "8ld"
#define S32_F "8ld"
#define X32_F "8lx"

#if defined (__ICCARM__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_USE_INCLUDES

#elif defined(__CC_ARM)

#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined(__GUNC__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#elif defined(__TASKING__)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef LWIP_LOGGER
#define LWIP_LOGGER(msg)
#endif

#define LWIP_RAND() ((u32_t)rand())
#define LWIP_PLATFORM_DIAG(x) {printf x;}

#define LWIP_PLATFORM_ASSERT(x) do { printf("Assertion \"%s\" failed at  \
        line %d in %s\n",x, __LINE__, __FILE__);} while(0)

#define LWIP_ERROR(message, expression, handler) do { if(!(expression)) { \
    printf("Assertion \"%s\" failed at line %d in %s\n", message, \
        __LINE__, __FILE__); handler;}} while(0)

#endif