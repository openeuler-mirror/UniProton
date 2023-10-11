#include "prt_typedef.h"
#include "pcie.h"
#include "pcie_config.h"

uintptr_t g_pcie_config_base_addr = NULL;

/* 支持更改默认配置空间地址 */
void pcie_config_base_addr_register(uintptr_t base_addr)
{
    g_pcie_config_base_addr = base_addr;
}

int pcie_device_cfg_read(uint32_t bdf, uint32_t offset, uint32_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint32_t *)(g_pcie_config_base_addr + addr);

    return 0;
}

int pcie_device_cfg_write(uint32_t bdf, uint32_t offset, uint32_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint32_t *)(g_pcie_config_base_addr + addr) = val;

    return 0;
}

int pcie_device_cfg_read_byte(uint32_t bdf, uint32_t offset, uint8_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint8_t *)(g_pcie_config_base_addr + addr);

    return 0;
}

int pcie_device_cfg_write_byte(uint32_t bdf, uint32_t offset, uint8_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint8_t *)(g_pcie_config_base_addr + addr) = val;

    return 0;
}

int pcie_device_cfg_read_word(uint32_t bdf, uint32_t offset, uint16_t *val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *val = *(uint16_t *)(g_pcie_config_base_addr + addr);

    return 0;
}

int pcie_device_cfg_write_word(uint32_t bdf, uint32_t offset, uint16_t val)
{
    uint32_t addr = PCI_CFG_ADDRESS_BY_BDF(bdf, offset);
    *(uint16_t *)(g_pcie_config_base_addr + addr) = val;

    return 0;
}