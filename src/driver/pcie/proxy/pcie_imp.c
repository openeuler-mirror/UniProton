
#include "pcie_extern.h"

/*

*/

int pci_register_driver(struct pci_driver* driver)
{
    int i;
    struct pci_driver_msg pci_driver_msg_d;

    memcpy_s(pci_driver_msg_d.name, PCI_DEVICE_NAME_MAX,
        driver->name, strlen(driver->name));

    for (i = 0; i < PCI_DEVICE_ID_TBL_MAX; i++) {
        pci_driver_msg_d.id_table[i] = driver->id_table[i];
        if (driver->id_table[i].vendor == 0) {
            break;
        }
    }
    send_msg((void *)(&pci_driver_msg_d), sizeof(pci_driver_msg_d));
}