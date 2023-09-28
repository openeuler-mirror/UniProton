
#ifndef __PCIE_H__
#define __PCIE_H__

#include "../list.h"
#include "prt_typedef.h"

struct pci_driver;

#define PCI_BDF(b, d, f) ((((b) & 0xff) << 8) | (((d) & 0x1f) << 3) | ((f) & 0x7))
#define PCI_BUS(bdf) (((devfn) >> 8) & 0xff)

#define PCI_DEVFN(slot, func)   ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn) (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn) ((devfn) & 0x07)

#define	PCI_ANY_ID              -1U
#define	PCI_VENDOR_ID_INTEL     0x8086
#define	PCI_VENDOR_ID_HUAWEI    0x19e5

#define	PCI_DEVICE(_vendor, _device)                        \
        .vendor = (_vendor), .device = (_device),           \
        .subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

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
    // TAILQ_HEAD(, pci_mmio_region) mmio;
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
    // int (*suspend) (struct pci_dev *dev, pm_message_t state); /* Device suspended */
    // int (*resume) (struct pci_dev *dev); /* Device woken up */
    // void (*shutdown) (struct pci_dev *dev); /* Device shutdown */
    // driver_t bsddriver;
    // devclass_t bsdclass;
    // struct device_driver driver;
    // const struct pci_error_handlers *err_handler;
    // bool isdrm;
    // int (*bsd_iov_init)(device_t dev, uint16_t num_vfs, const nvlist_t *pf_config);
    // void (*bsd_iov_uninit)(device_t dev);
    // int (*bsd_iov_add_vf)(device_t dev, uint16_t vfnum, const nvlist_t *vf_config);
};

int pci_frame_init(void);

int pci_driver_register(struct pci_driver *pci_drv);

#if 0

typedef union {
    struct {
        U32 function : 3;
        U32 device : 5;
        U32 bus : 8;
    } bdf;
    U16 value;
} BDF_U;

#define BDF_GET_FROM_DD(dd) (BDF_U)((U16)(dd))

#define BAR_NUM 6
typedef struct tagPCI_DEVICE {
    U16 bdf;
    uintptr_t bar_phy[BAR_NUM];
    uintptr_t bar_virt[BAR_NUM];
    U32 bar_size[BAR_NUM];
    void *pci_data;
    struct tagPCI_DEVICE *prev;
    struct tagPCI_DEVICE *next;
} pci_device_t;
#endif

#endif