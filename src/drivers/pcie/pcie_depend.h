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
 * Create: 2023-10-17
 * Description: PCIE功能 依赖其他模块实现的接口
 */

#ifndef __PCIE_DEPEND_H__
#define __PCIE_DEPEND_H__

#include "prt_typedef.h"

extern bool pci_bus_accessible(uint32_t bus_no);

#define UNIPROTON_NODE_PATH "/run/pci_uniproton/"
extern int proxybash_exec_lock(char *cmdline, char *result_buf, unsigned int buf_len);

#endif