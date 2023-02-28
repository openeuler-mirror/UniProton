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
#ifndef OS_BUILDEF_COMMON_H
#define OS_BUILDEF_COMMON_H

/* the endian definition */
#define OS_LITTLE_ENDIAN    0x1234
#define OS_BIG_ENDIAN       0x4321

/* To define OS_HARDWARE_PLATFORM */
/* 编译器有个bug, 未定义的宏的数值默认是0，所以不用使用'0' */
#define OS_CORTEX_M4        0x01
#define OS_ARMV8            0x02
#define OS_PLATFORM_INVALID 0x03

/* To define OS_CPU_TYPE */
/* 编译器有个bug, 未定义的宏的数值默认是0，所以不用使用'0' */
#define OS_STM32F407        0x01
#define OS_RASPI4           0x02
#define OS_HI3093           0x03
#define OS_CPU_TYPE_INVALID 0x04

#ifndef INIT_SEC_L4_TEXT
#define INIT_SEC_L4_TEXT 
#endif

#ifndef RESET_SEC_DATA
#define RESET_SEC_DATA __attribute__((section(".os.init.data")))
#endif

/* 必须方案sre_buildef的头文件中，否则会出现宏开关不生效的问题 */
#ifdef YES
#undef YES
#endif
#define YES 1

#ifdef NO
#undef NO
#endif
#define NO 0

#endif /* OS_BUILDEF_COMMON_H */
