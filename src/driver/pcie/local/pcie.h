
#ifndef __PCIE_H__
#define __PCIE_H__

#include "../list.h"
#include "prt_typedef.h"

struct pci_driver;

#define PCI_BDF(b, d, f) ((((b) & 0xff) << 8) | (((d) & 0x1f) << 3) | ((f) & 0x7))
#define PCI_BUS(bdf) (((bdf) >> 8) & 0xff)
#define PCI_DEVFN_FROM_BDF(bdf) ((bdf) & 0xff)

#define PCI_DEVFN(slot, func)   ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn) (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn) ((devfn) & 0x07)

#define	PCI_ANY_ID              -1U
#define	PCI_VENDOR_ID_INTEL     0x8086
#define	PCI_VENDOR_ID_HUAWEI    0x19e5

#define	PCI_DEVICE(_vendor, _device)                        \
        .vendor = (_vendor), .device = (_device),           \
        .subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

/* For PCI devices, the region numbers are assigned this way: */
enum {
    /* #0-5: standard PCI resources */
    PCI_STD_RESOURCES,
    PCI_STD_RESOURCE_END = 5,

    /* Total resources associated with a PCI device */
    PCI_NUM_RESOURCES,
    /* Preserve this for compatibility */
    DEVICE_COUNT_RESOURCE = PCI_NUM_RESOURCES,
};

struct resource {
    uint64_t start;
    uint64_t end;
    uint32_t flags;
};

// pci设备结构体
struct pci_dev {
    // struct device dev;
    struct list_head links;
    struct pci_driver *pdrv;
    // struct pci_bus *bus;
    uint16_t device;
    uint16_t vendor;
    uint16_t subsystem_vendor;
    uint16_t subsystem_device;
    unsigned int irq;
    unsigned int bus_no;
    unsigned int devfn;
    uint32_t class;
    uint8_t revision;
    bool msi_enabled;
    struct resource resource[DEVICE_COUNT_RESOURCE]; /* I/O and memory regions */
};

struct pci_device_id {
    uint32_t vendor;
    uint32_t device;
    uint32_t subvendor;
    uint32_t subdevice;
    uint32_t class;
    uint32_t class_mask;
    uintptr_t driver_data;
};

// pci 驱动结构体
struct pci_driver {
    struct list_head links;
    char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
};

int pci_frame_init(uint64_t pci_cfg_base);

int pci_driver_register(struct pci_driver *pci_drv);

#endif