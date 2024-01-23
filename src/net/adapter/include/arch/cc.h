/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-09-21
 * Description: 网络
 */

#ifndef LWIP_PORTING_CC_H
#define LWIP_PORTING_CC_H

#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif

#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "securec.h"

#ifdef htons
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#endif

#define SOCKLEN_T_DEFINED
#define SA_FAMILY_T_DEFINED
#define IN_PORT_T_DEFINED

#define LWIP_TIMEVAL_PRIVATE    0
#define LWIP_ERRNO_STDINCLUDE
#define LWIP_SOCKET_STDINCLUDE

#define LWIP_DNS_API_DEFINE_ERRORS    0
#define LWIP_DNS_API_DEFINE_FLAGS     0
#define LWIP_DNS_API_DECLARE_STRUCTS  0
#define LWIP_DNS_API_DECLARE_H_ERRNO  0

#ifndef __SIZEOF_POINTER__
#define __SIZEOF_POINTER__ 4   // 32 bit system
#endif

#define OS_TASK_STATUS_DETACHED   0x0100  // reserved

#if defined(__arm__) && defined(__ARMCC_VERSION)
    /* Keil uVision4 tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __align(n)
#elif defined (__IAR_SYSTEMS_ICC__)
    /* IAR Embedded Workbench tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    // #error NEEDS ALIGNED
#else
    /* GCC tools (CodeSourcery) */
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __attribute__((aligned (n)))
#endif

#define LWIP_RAND rand

#ifndef LWIP_LOGGER
#define LWIP_LOGGER(msg)
#endif

extern void OsLwipLogPrintf(const char *fmt, ...);
#define LWIP_PLATFORM_DIAG(vars) OsLwipLogPrintf vars
#define LWIP_PLATFORM_ASSERT(x) do { \
        LWIP_PLATFORM_DIAG(("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__)); \
    } while (0)

#define init_waitqueue_head(...)
#define poll_check_waiters(...)

#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif

#endif /* LWIP_PORTING_CC_H */
