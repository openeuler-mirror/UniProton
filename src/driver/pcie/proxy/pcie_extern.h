

typedef unsigned int __u32;
typedef unsigned long kernel_ulong_t;

#define PCI_VENDOR_ID_INTEL 0x8086
#define PCI_VENDOR_ID_HUAWEI 0x19e5

#define PCI_DEVICE_NAME_MAX   64
#define PCI_DEVICE_ID_TBL_MAX 12
struct pci_device_id {
	__u32 vendor, device;		/* Vendor and device ID or PCI_ANY_ID*/
	__u32 subvendor, subdevice;	/* Subsystem ID's or PCI_ANY_ID */
	__u32 classd, class_mask;	/* (class,subclass,prog-if) triplet */
	kernel_ulong_t driver_data;	/* Data private to the driver */
};

#define PCI_ANY_ID (~0)

#define PCI_VDEVICE(vend, dev) \
	.vendor = PCI_VENDOR_ID_##vend, .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID, 0, 0

struct pci_driver_msg {
    const char name[PCI_DEVICE_NAME_MAX];
    const struct pci_device_id id_table[PCI_DEVICE_ID_TBL_MAX];
};

struct pci_driver {
	const char *name;
	const struct pci_device_id *id_table;
	int  (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
	void (*remove)(struct pci_dev *dev);
};