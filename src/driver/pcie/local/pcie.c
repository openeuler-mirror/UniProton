
#include "cpu_config.h"
#include "prt_typedef.h"
#include "pcie.h"
#include "pcie_config.h"

LIST_HEAD(g_pcie_device_list_head);

LIST_HEAD(g_pcie_driver_list_head);

int pci_match_dev(struct pci_device_id *pci_dev_id_tbl, uint32_t bdf,
    struct pci_device_id **pci_dev_id);
struct pci_dev *pci_dev_create_by_bdf(uint32_t bdf);
int pci_dev_add(struct pci_dev *pdev);

/* 全局变量的初始化， 供后期用户注册驱动程序 */
int pci_frame_init(void)
{
    pcie_config_base_addr_register(MMU_ECAM_ADDR);
    return 0;
}

/* 根据 pci_drv 中的 pci_device_id 匹配所有pci设备，若果能匹配到，则执行挂接probe函数 */
int pci_driver_register(struct pci_driver *pci_drv)
{
    int ret;
    int b, d, f;
    uint32_t bdf;
    struct pci_device_id *pci_dev_id_tbl;
    struct pci_device_id *pci_dev_id;
    struct pci_dev *pci_device;

    if (pci_drv == NULL) {
        return OS_OK;
    }

    pci_dev_id_tbl = pci_drv->id_table;
    for (b = 0; b < PCI_BUS_NUM_MAX; b++) {
        for (d = 0; d < PCI_DEIVCE_NUM_MAX; d++) {
            for (f = 0; f < PCI_FUNCTION_NUM_MAX; f++) {
                bdf = PCI_BDF(b, d, f);
                ret = pci_match_dev(pci_dev_id_tbl, bdf, &pci_dev_id);
                if (ret == 0) {
                    continue;
                }
                pci_device = pci_dev_create_by_bdf(bdf);
                if (pci_device == NULL) {
                    return OS_FAIL;
                }
                pci_device->pdrv = pci_drv;
                ret = pci_dev_add(pci_device);
                ret = pci_drv->probe(pci_device, pci_dev_id);
            }
        }
    }

    return ret;
}

int pci_match_dev(struct pci_device_id *pci_dev_id_tbl, uint32_t bdf,
    struct pci_device_id **pci_dev_id)
{
    uint32_t ret;
    uint32_t vendor, device, subvendor, subdevice, headertype;
    struct pci_device_id *pdid;

    ret = pcie_device_cfg_read_byte(bdf, PCI_HEADER_TYPE, &headertype);
    if ((headertype & PCI_HEADER_TYPE_MASK) != PCI_HEADER_TYPE_NORMAL) {
        return FALSE; /* 非终端设备， 桥设备或者cardbus */
    }

    ret = pcie_device_cfg_read_word(bdf, PCI_VENDOR_ID, &vendor);
    ret = pcie_device_cfg_read_word(bdf, PCI_DEVICE_ID, &device);
    if (vendor == 0xffff && device == 0xffff) {
        return FALSE; /* 该槽位没有设备 */
    }

    ret = pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_VENDOR_ID, &subvendor);
    ret = pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_ID, &subdevice);

    for (pdid = pci_dev_id_tbl; (pdid->vendor | pdid->device) != 0; pdid++) {
        if ( (pdid->device == PCI_ANY_ID || pdid->device == device) &&
            (pdid->vendor == PCI_ANY_ID || pdid->vendor == vendor) &&
            (pdid->subvendor == PCI_ANY_ID || pdid->subvendor == subvendor) &&
            (pdid->subdevice == PCI_ANY_ID || pdid->subdevice == subdevice)
        ) {
            *pci_dev_id = pdid; /* 记录该id,并返回匹配OK */
            return TRUE;
        }
    }
    return FALSE;
}

static inline unsigned long decode_bar(uint32_t bar)
{
    uint32_t mem_type;
    unsigned long flags;

    if ((bar & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
        flags = bar & ~PCI_BASE_ADDRESS_IO_MASK;
        flags |= IORESOURCE_IO;
        return flags;
    }

    flags = bar & ~PCI_BASE_ADDRESS_MEM_MASK;
    flags |= IORESOURCE_MEM;
    if (flags & PCI_BASE_ADDRESS_MEM_PREFETCH)
        flags |= IORESOURCE_PREFETCH;

    mem_type = bar & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
    switch (mem_type) {
    case PCI_BASE_ADDRESS_MEM_TYPE_32:
        break;
    case PCI_BASE_ADDRESS_MEM_TYPE_1M:
        /* 1M mem BAR treated as 32-bit BAR */
        break;
    case PCI_BASE_ADDRESS_MEM_TYPE_64:
        flags |= IORESOURCE_MEM_64;
        break;
    default:
        /* mem unknown type treated as 32-bit BAR */
        break;
    }
    return flags;
}

static uint64_t pci_size(uint64_t base, uint64_t maxbase, uint64_t mask)
{
    uint64_t size = mask & maxbase;	/* Find the significant bits */
    if (!size)
        return 0;

    /*
    * Get the lowest of them to find the decode size, and from that
    * the extent.
    */
    size = (size & ~(size-1)) - 1; 

    /*
    * base == maxbase can be valid only if the BAR has already been
    * programmed with all 1s.
    */
    if (base == maxbase && ((base | size) & mask) != mask)
        return 0;

    return size;
}

int pci_read_base(struct pci_dev *pdev, struct resource *res, uint32_t bar)
{
    uint32_t bdf;
    uint32_t l = 0, sz = 0, mask = ~0;
    uint64_t l64, sz64, mask64;

    bdf = (pdev->bus_no << 8) | pdev->devfn;

    pcie_device_cfg_read_dword(bdf, PCI_REG_BAR(bar), &l);
    pcie_device_cfg_write_dword(bdf, PCI_REG_BAR(bar), mask);
    pcie_device_cfg_read_dword(bdf, PCI_REG_BAR(bar), &sz);
    pcie_device_cfg_write_dword(bdf, PCI_REG_BAR(bar), l);

    /*
    * All bits set in sz means the device isn't working properly.
    * If the BAR isn't implemented, all bits must be 0.  If it's a
    * memory BAR or a ROM, bit 0 must be clear; if it's an io BAR, bit
    * 1 must be clear.
    */
    if (sz == 0xffffffff)
        sz = 0;

    /*
    * I don't know how l can have all bits set.  Copied from old code.
    * Maybe it fixes a bug on some ancient platform.
    */
    if (l == 0xffffffff)
        l = 0;

    res->flags = decode_bar(l);
    res->flags |= IORESOURCE_SIZEALIGN;
    if (res->flags & IORESOURCE_IO) {
        l64 = l & PCI_BASE_ADDRESS_IO_MASK;
        sz64 = sz & PCI_BASE_ADDRESS_IO_MASK;
        mask64 = PCI_BASE_ADDRESS_IO_MASK & (uint32_t)IO_SPACE_LIMIT;
    } else {
        l64 = l & PCI_BASE_ADDRESS_MEM_MASK;
        sz64 = sz & PCI_BASE_ADDRESS_MEM_MASK;
        mask64 = (uint32_t)PCI_BASE_ADDRESS_MEM_MASK;
    }

    if (res->flags & IORESOURCE_MEM_64) {
        pcie_device_cfg_read_dword(bdf, PCI_REG_BAR(bar + 1), &l);
        pcie_device_cfg_write_dword(bdf, PCI_REG_BAR(bar + 1), ~0);
        pcie_device_cfg_read_dword(bdf, PCI_REG_BAR(bar + 1), &sz);
        pcie_device_cfg_write_dword(bdf, PCI_REG_BAR(bar + 1), l);

        l64 |= ((uint64_t)l << 32);
        sz64 |= ((uint64_t)sz << 32);
        mask64 |= ((uint64_t)~0 << 32);
    }

    sz64 = pci_size(l64, sz64, mask64);
    if (!sz64) {
        res->flags = 0;
        printf("bar 0x%x: invalid BAR (can't size)\n", bar);
    }

    res->start = l64;
    res->end = l64 + sz64;

    return (res->flags & IORESOURCE_MEM_64) ? 1 : 0;
}

// malloc and init pci_dev
struct pci_dev *pci_dev_create_by_bdf(uint32_t bdf)
{
    int bar;
    struct pci_dev *pdev;
    uint32_t class_revision, bar_value, orig_cmd;

    pdev = (struct pci_dev*)malloc(sizeof(struct pci_dev));
    if (pdev == NULL) {
        return NULL;
    }

    pcie_device_cfg_read_word(bdf, PCI_VENDOR_ID, &(pdev->vendor));
    pcie_device_cfg_read_word(bdf, PCI_DEVICE_ID, &(pdev->device));
    pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_VENDOR_ID, &(pdev->subsystem_vendor));
    pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_ID, &(pdev->subsystem_device));
    pcie_device_cfg_read_word(bdf, PCI_CLASS_REVISION, &class_revision);
    pdev->class = class_revision & 0xff;
    pdev->revision = class_revision >> 8;

    pdev->bus_no = PCI_BUS(bdf);
    pdev->devfn = PCI_DEVFN_FROM_BDF(bdf);

    /* 这里需要配置 PCI_COMMAND 关闭 io 和 mem 空间访问，因为需要读写bar */
    pcie_device_cfg_read_word(bdf, PCI_COMMAND, &orig_cmd);
    if (orig_cmd & PCI_COMMAND_DECODE_ENABLE) {
        pcie_device_cfg_write_word(bdf, PCI_COMMAND,
            orig_cmd & ~PCI_COMMAND_DECODE_ENABLE);
    }

    for (bar = 0; bar < DEVICE_COUNT_RESOURCE; bar++) {
        /* 如果地址为64bit, 会占用2个bar寄存器, 此时返回1, 否则返回0 */
        bar += pci_read_base(pdev, &(pdev->resource[bar]), bar);
    }

    if (orig_cmd & PCI_COMMAND_DECODE_ENABLE)
        pcie_device_cfg_write_word(bdf, PCI_COMMAND, orig_cmd);

    return pdev;
}

int pci_dev_add(struct pci_dev *pdev)
{
    list_add_tail(&(pdev->links), &g_pcie_device_list_head);
    if (pdev->pdrv->links.prev == NULL) { /* 避免重复push同一driver */
        list_add_tail(&(pdev->pdrv->links), &g_pcie_driver_list_head);
    }
    return OS_OK;
}
