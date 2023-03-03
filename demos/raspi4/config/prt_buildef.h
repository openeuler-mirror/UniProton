/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: the common part buildef.h
 */
#ifndef PRT_BUILDEF_H
#define PRT_BUILDEF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OS_ARCH_ARMV8

#define OS_GIC_VER 2

#ifndef OS_HARDWARE_PLATFORM
#define OS_HARDWARE_PLATFORM OS_ARMV8
#endif

#ifndef OS_CPU_TYPE
#define OS_CPU_TYPE OS_RASPI4
#endif

#define OS_MAX_CORE_NUM 4

#define OS_THIS_CORE 3

#ifndef OS_BYTE_ORDER
#define OS_BYTE_ORDER OS_LITTLE_ENDIAN
#endif

#define OS_CACHE_LINE_SIZE 64

#define OS_OPTION_EVENT

#define OS_OPTION_QUEUE

#define OS_OPTION_HWI_PRIORITY

#define OS_OPTION_HWI_ATTRIBUTE

#define OS_OPTION_SYS_TIME_USR

#define OS_OPTION_TASK

#define OS_OPTION_TASK_DELETE

#define OS_OPTION_TASK_SUSPEND

#define OS_OPTION_TASK_INFO

#define OS_OPTION_TASK_YIELD

#define OS_TSK_PRIORITY_HIGHEST 0

#define OS_TSK_PRIORITY_LOWEST 63

#define OS_TSK_NUM_OF_PRIORITIES 64

#define OS_TSK_CORE_BYTES_IN_PID 2

#define OS_OPTION_TICK

#define OS_OPTION_CPUP_WARN

#define OS_OPTION_RND

#define OS_OPTION_POSIX

#define OS_POSIX_TYPE_NEWLIB

#define OS_OPTION_OPENAMP

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

/* common macros's definitions */
#include "prt_buildef_common.h"

#endif
