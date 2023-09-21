
#include "cpu_config.h"
#include "prt_typedef.h"
#include "pcie.h"
#include "pcie_config.h"

void pci_dev_config_init_by_bdf(pci_device_t *device_node);

uintptr_t g_pcie_config_base_addr = NULL;

pci_device_t pci_dev_list_head = { 
    .prev = NULL,
    .next = NULL,
};

void pci_dev_list_insert(pci_device_t *device_node)
{
    pci_device_t *new_device_node = (pci_device_t *)malloc(sizeof(pci_device_t));
    *new_device_node = *device_node;
    new_device_node->prev = &pci_dev_list_head;
    new_device_node->next = pci_dev_list_head.next;
    pci_dev_list_head.next = new_device_node;
}

void pcie_config_base_addr_register(uintptr_t base_addr)
{
    g_pcie_config_base_addr = base_addr;
}

U32 pcie_device_cfg_read(BDF_U bdf, U32 offset)
{
    U32 addr =
        PCI_ECAM_ADDRESS(bdf.bdf.bus, bdf.bdf.device, bdf.bdf.function, offset);

    return *(U32 *)(g_pcie_config_base_addr + addr);
}

void pcie_device_cfg_write(BDF_U bdf, U32 offset, U32 data)
{
    U32 addr =
        PCI_ECAM_ADDRESS(bdf.bdf.bus, bdf.bdf.device, bdf.bdf.function, offset);

    *(U32 *)(g_pcie_config_base_addr + addr) = data;
}

U32 pcie_scan_device(U16 device_id, U16 vendor_id)
{
    U32 dev;
    U32 bus;
    BDF_U bdf = { .value = 0 };
    U16 vendor_id_r;
    U16 device_id_r;
    pci_device_t device_node;

    for (bus = 0; bus < PCI_BUS_NUM_MAX; bus++) {
        for (dev = 0; dev < PCI_DEIVCE_NUM_MAX; dev++) {
            bdf.bdf.bus = bus;
            bdf.bdf.device = dev;
            vendor_id_r = pcie_device_cfg_read(bdf, PCI_VENDOR_ID);
            device_id_r = pcie_device_cfg_read(bdf, PCI_DEVICE_ID);
            if (vendor_id_r == vendor_id && device_id_r == device_id) {
                device_node.bdf = bdf.value;
                pci_dev_config_init_by_bdf(&device_node);
                pci_dev_list_insert(&device_node);
                return (U32)(bdf.value);
            }
        }
    }

    return -1;
}

void pci_dev_config_init_by_bdf(pci_device_t *device_node)
{
    U32 bar_id;
    for (bar_id = 0; bar_id < BAR_NUM; bar_id++) {
        device_node->bar_phy[bar_id] = pcie_device_cfg_read(
            (BDF_U)(device_node->bdf), (PCI_BASE_ADDRESS_0 + (bar_id * 4)));
    }
    
}

uintptr_t pci_resource_start(BDF_U bdf, U32 bar_id)
{
    U32 bar0, bar0_bk;
    U32 bar1, bar1_bk;
    U32 mem_reg;
    U32 len;
    if (bar_id >= 6) {
        return NULL;
    }
    bar0 = pcie_device_cfg_read(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)));

    mem_reg = bar0 & 0x1; // 0 mem; 1 reg;
    if (mem_reg) { /* reg */
        bar0_bk = bar0;
        pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), 0xffffffff);
        bar0 = pcie_device_cfg_read(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)));
        len = ~(bar0 & 0xfffffffc);
        pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), bar0_bk);
    } else { /* memory */
        if (bar0 & 0x4) { // 64bit memory 地址
            bar0_bk = bar0;
            pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), 0xffffffff);
            bar0 = pcie_device_cfg_read(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)));
            len = ~(bar0 & 0xfffffff0);
            pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), bar0_bk);
            bar1 = pcie_device_cfg_read(bdf, (PCI_BASE_ADDRESS_0 + (bar_id + 1 * 4)));
        } else { // 32bit memory 地址
            bar0_bk = bar0;
            pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), 0xffffffff);
            bar0 = pcie_device_cfg_read(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)));
            len = ~(bar0 & 0xfffffff0);
            pcie_device_cfg_write(bdf, (PCI_BASE_ADDRESS_0 + (bar_id * 4)), bar0_bk);
        }
    }
}