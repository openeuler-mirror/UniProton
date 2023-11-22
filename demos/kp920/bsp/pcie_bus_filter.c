#include "prt_buildef.h"
#include "prt_typedef.h"

struct pcie_bus_range_s {
    uint64_t start;
    uint64_t end;
    uint32_t bus_start;
    uint32_t bus_end;
};

/* 这个表从linux侧/proc/iomem解析得到，待引入文件代理功能后改用读文件方式获取 */
const struct pcie_bus_range_s g_pci_ecam_range[] = {
    { 0xd0000000, 0xd3ffffff, 0x00, 0x3f },
    { 0xd7400000, 0xd75fffff, 0x74, 0x75 },
    { 0xd7800000, 0xd79fffff, 0x78, 0x79 },
    { 0xd7a00000, 0xd7afffff, 0x7a, 0x7a },
    { 0xd7b00000, 0xd7bfffff, 0x7b, 0x7b },
    { 0xd7c00000, 0xd7dfffff, 0x7c, 0x7d },
    { 0xd8000000, 0xd9ffffff, 0x80, 0x9f },
    { 0xdb400000, 0xdb5fffff, 0xb4, 0xb5 },
    { 0xdb800000, 0xdb9fffff, 0xb8, 0xb9 },
    { 0xdba00000, 0xdbafffff, 0xba, 0xba },
    { 0xdbb00000, 0xdbbfffff, 0xbb, 0xbb },
    { 0xdbc00000, 0xdbdfffff, 0xbc, 0xbd },
};

uint32_t g_pci_ecam_range_num = sizeof(g_pci_ecam_range) / sizeof(g_pci_ecam_range[0]);

/* PCIE的配置空间，不连续时，有些bus的配置空间访问出发异常，需要跳过 */
bool pci_bus_accessible(uint32_t bus_no)
{
    int i;
    for (i = 0; i < g_pci_ecam_range_num; i++) {
        if (bus_no >= g_pci_ecam_range[i].bus_start &&
            bus_no <= g_pci_ecam_range[i].bus_end) {
            return true;
        }
    }

    return false;
}