
#include "pcie.h"

U32 pcie_scan_device(U16 device_id, U16 vendor_id);

#define PCI_VENDOR_ID PCI_VENDOR_ID_HUAWEI
#define PCI_DEVICE_ID 0x1711

#define error -1
#define ok 0

static char g_hpm_driver_name[] = "huawei_pci_model";
static struct pci_device_id g_hpm_pci_dev_tbl[] = {
    { PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID) },
    { PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID + 1) },
    { PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID + 2) },
    { 0, 0 },
};

int hpm_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    


    return 0;
}

void hpm_remove(struct pci_dev *dev)
{
    return;
}

static struct pci_driver hpm_driver = {
    .name       = g_hpm_driver_name,
    .id_table   = g_hpm_pci_dev_tbl,
    .probe      = hpm_probe,
    .remove     = hpm_remove,
};

void test_pcie(void)
{
    int ret;

    // 由系统初始化调用
    ret = pci_frame_init();
    if (ret != ok) {
        return;
    }

    // 根据 dev_tbl 查找所有设备， 并调用 挂接的 probe 函数
    ret = pci_driver_register(&hpm_driver);
    if (ret != ok) {
        return;
    }

    return;
}
