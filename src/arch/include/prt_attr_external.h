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
 * Description: 属性宏相关内部头文件
 */
#ifndef PRT_ATTR_EXTERNAL_H
#define PRT_ATTR_EXTERNAL_H

#include "prt_buildef.h"

#if defined(OS_ARCH_ARMV7_M)
#include "../cpu/armv7-m/common/os_attr_armv7_m_external.h"
#endif

#if defined(OS_ARCH_ARMV8)
#include "../cpu/armv8/common/os_attr_armv8_external.h"
#endif

#if defined(OS_ARCH_X86_64)
#include "../cpu/x86_64/common/os_attr_x86_64_external.h"
#endif

#if defined(OS_ARCH_RISCV64)
#include "../cpu/riscv64/common/os_attr_riscv64_external.h"
#endif

#endif /* PRT_ATTR_EXTERNAL_H */
