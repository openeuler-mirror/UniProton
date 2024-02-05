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
 * Create: 2024-01-14
 * Description: RISCV64Gclient制器PLIC对内驱动头文件
 */

#ifndef PRT_HW_CLIENT_H
#define PRT_HW_CLIENT_H

#include "prt_buildef_common.h"
#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_attr_external.h"
#include "../riscv.h"

#if (OS_CPU_TYPE == OS_RV64_VIRT)
#include "./board/qemu_rv64virt/platform.h"
#endif


extern void OsTaskTrap(void);


#endif
