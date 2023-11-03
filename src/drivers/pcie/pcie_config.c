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
 * Description: PCIE功能
 */

#include "prt_typedef.h"
#include "pcie.h"
#include "pcie_config.h"

uintptr_t g_pcie_config_base_addr = NULL;

/* 支持更改默认配置空间地址 */
void pcie_config_base_addr_register(uintptr_t base_addr)
{
    g_pcie_config_base_addr = base_addr;
}

void pcie_device_cfg_read(uint32_t bdf, uint32_t offset, uint32_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint32_t *)(g_pcie_config_base_addr + addr);
}

void pcie_device_cfg_write(uint32_t bdf, uint32_t offset, uint32_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint32_t *)(g_pcie_config_base_addr + addr) = val;
}

void pcie_device_cfg_read_byte(uint32_t bdf, uint32_t offset, uint8_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint8_t *)(g_pcie_config_base_addr + addr);
}

void pcie_device_cfg_write_byte(uint32_t bdf, uint32_t offset, uint8_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint8_t *)(g_pcie_config_base_addr + addr) = val;
}

void pcie_device_cfg_read_halfword(uint32_t bdf, uint32_t offset, uint16_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint16_t *)(g_pcie_config_base_addr + addr);;
}

void pcie_device_cfg_write_halfword(uint32_t bdf, uint32_t offset, uint16_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint16_t *)(g_pcie_config_base_addr + addr) = val;
}