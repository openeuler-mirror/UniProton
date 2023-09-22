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
 * Create: 2023-06-25
 * Description: 属性宏相关内部头文件
 */
#ifndef OS_ATTR_X86_64_EXTERNAL_H
#define OS_ATTR_X86_64_EXTERNAL_H

/* 定义操作系统的代码数据分段 */

#define OS_SEC_LX0_TEXT
#define OS_SEC_L0_TEXT
#define OS_SEC_TEXT
#define OS_SEC_L2_TEXT
#define OS_SEC_L4_TEXT
#define OS_SEC_LX_TEXT
#define OS_SEC_DATA
#define OS_SEC_L4_DATA

#ifndef OS_SEC_BSS
#define OS_SEC_BSS __attribute__((section(".os.bss")))
#endif

#ifndef OS_SEC_L4_BSS
#define OS_SEC_L4_BSS __attribute__((section(".os.bss")))
#endif

#define OS_SEC_L4_INSTSH_DATA
#define SHELL_SEC_L4_TEXT
#define SHELL_SEC_L4_DATA

#ifndef OS_SEC_L0_OPTION_TEXT
#define OS_SEC_L0_OPTION_TEXT OS_SEC_L0_TEXT
#endif

#ifndef OS_SEC_L1_OPTION_TEXT
#define OS_SEC_L1_OPTION_TEXT OS_SEC_TEXT
#endif

#ifndef OS_SEC_L2_OPTION_TEXT
#define OS_SEC_L2_OPTION_TEXT OS_SEC_L2_TEXT
#endif

#endif /* OS_ATTR_X86_64_EXTERNAL_H */
