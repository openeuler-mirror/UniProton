#include "prt_typedef.h"
#include "ivshmem_demo.h"
#include "prt_mem.h"

static inline U8 mmio_read8(void *address)
{
    return *(volatile U8 *)address;
}

static inline U16 mmio_read16(void *address)
{
    return *(volatile U16 *)address;
}

static inline U32 mmio_read32(void *address)
{
    return *(volatile U32 *)address;
}

static inline void synchronization_barrier(void)
{
    dsb(ish);
}

static inline void mmio_write8(void *address, U8 value)
{
    *(volatile U8 *)address = value;
}

static inline void mmio_write16(void *address, U16 value)
{
    *(volatile U16 *)address = value;
}

static inline void mmio_write32(void *address, U32 value)
{
    *(volatile U32 *)address = value;
}

void *zalloc(unsigned long size, unsigned long align)
{
    void *base = PRT_MemAlloc(0, 0, size); //diff
    memset_s(base, size, 0, size);

    return base;
}

void map_range(void *start, unsigned long size, enum map_type map_type)
{
    U64 vaddr, pmd_entry;
    unsigned pgd_index;
    U64 *pmd;

    vaddr = (unsigned long)start;

    size += (vaddr & ~HUGE_PAGE_MASK) + HUGE_PAGE_SIZE - 1;
    size &= HUGE_PAGE_MASK;

    while (size) {
        pgd_index = PGD_INDEX(vaddr);
        if (!(page_directory[pgd_index] & LONG_DESC_TABLE)) {
            pmd = zalloc(PAGE_SIZE, PAGE_SIZE);
            /* ensure the page table walker will see the zeroes */
            synchronization_barrier();

            page_directory[pgd_index] = (unsigned long)pmd | LONG_DESC_TABLE;
        } else {
            pmd = (U64*)(unsigned long)(page_directory[pgd_index] & ~LONG_DESC_TABLE);
        }

        pmd_entry = vaddr & HUGE_PAGE_MASK;
        pmd_entry |= LATTR_AF | LATTR_INNER_SHAREABLE | LATTR_AP_RW_EL1 | LONG_DESC_BLOCK;
        if (map_type == MAP_CACHED)
            pmd_entry |= LATTR_MAIR(0);
        else
            pmd_entry |= LATTR_MAIR(1);

        pmd[PMD_INDEX(vaddr)] = pmd_entry;

        size -= HUGE_PAGE_SIZE;
        vaddr += HUGE_PAGE_SIZE;
    }

    /*
     * As long es we only add entries and do not modify entries, a
     * synchronization barrier is enough to propagate changes. Otherwise we
     * need to flush the TLB.
     */
    synchronization_barrier();
}

void pci_init(void)
{
    void *mmcfg = (void *)(unsigned long)comm_region->pci_mmconfig_base;

    if (mmcfg)
        map_range(mmcfg, 0x100000, MAP_UNCACHED);
}

static void *pci_get_device_mmcfg_base(U16 bdf)
{
    void *mmcfg = (void *)(unsigned long)comm_region->pci_mmconfig_base;

    return mmcfg + ((unsigned long)bdf << 12);
}

U32 pci_read_config(U16 bdf, unsigned int addr, unsigned int size)
{
    void *cfgaddr = pci_get_device_mmcfg_base(bdf) + addr;

    switch (size) {
        case 1:
            return mmio_read8(cfgaddr);
        case 2:
            return mmio_read16(cfgaddr);
        case 4:
            return mmio_read32(cfgaddr);
        default:
            return -1;
    }
}

U64 pci_cfg_read64(U16 bdf, unsigned int addr)
{
    return pci_read_config(bdf, addr, 4) | ((U64)pci_read_config(bdf, addr + 4, 4) << 32);
}

void pci_write_config(U16 bdf, unsigned int addr, U32 value, unsigned int size)
{
    void *cfgaddr = pci_get_device_mmcfg_base(bdf) + addr;

    switch (size) {
        case 1:
            mmio_write8(cfgaddr, value);
            break;
        case 2:
            mmio_write16(cfgaddr, value);
            break;
        case 4:
            mmio_write32(cfgaddr, value);
            break;
    }
}

int pci_find_device(U16 vendor, U16 device, U16 start_bdf)
{
    unsigned int bdf;
    U16 id;

    for (bdf = start_bdf; bdf < 0x10000; bdf++) {
        id = pci_read_config(bdf, PCI_CFG_VENDOR_ID, 2);
        if (id == PCI_ID_ANY || (vendor != PCI_ID_ANY && vendor != id))
            continue;
        if (device == PCI_ID_ANY ||
            pci_read_config(bdf, PCI_CFG_DEVICE_ID, 2) == device)
            return bdf;
    }
    return -1;
}

int pci_find_cap(U16 bdf, U16 cap)
{
    U8 pos = PCI_CFG_CAP_PTR - 1;

    if (!(pci_read_config(bdf, PCI_CFG_STATUS, 2) & PCI_STS_CAPS))
        return -1;

    while (1) {
        pos = pci_read_config(bdf, pos + 1, 1);
        if (pos == 0)
            return -1;
        if (pci_read_config(bdf, pos, 1) == cap)
            return pos;
    }
}
