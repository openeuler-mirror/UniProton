
#include "pcie_extern.h"

struct pci_device_id pci_dev_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xa120) },
    { PCI_VDEVICE(HUAWEI, 0xa121) },
    { PCI_VDEVICE(HUAWEI, 0x1710) },
    { PCI_VDEVICE(HUAWEI, 0x1711) },
    { 0, } /* required last entry */
};

char *pci_dev_name = "huawei_pci_model";

void pci_model_probe(void)
{

}

void pci_model_remove(void)
{

}

struct pci_driver pci_model_driver = {
    .name = pci_dev_name,
    .id_table = pci_dev_tbl,
    .probe = pci_model_probe,
    .remove = pci_model_remove,
}

void huawei_pci_model_init()
{
    pci_register_driver(&pci_model_driver);
}
