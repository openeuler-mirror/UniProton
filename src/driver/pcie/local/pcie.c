
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

// malloc and init pci_dev
struct pci_dev *pci_dev_create_by_bdf(uint32_t bdf)
{
    int ret;
    struct pci_dev *pdev;
    uint32_t class_revision;

    pdev = (struct pci_dev*)malloc(sizeof(struct pci_dev));
    if (pdev == NULL) {
        return NULL;
    }

    ret = pcie_device_cfg_read_word(bdf, PCI_VENDOR_ID, &(pdev->vendor));
    ret = pcie_device_cfg_read_word(bdf, PCI_DEVICE_ID, &(pdev->device));
    ret = pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_VENDOR_ID, &(pdev->subsystem_vendor));
    ret = pcie_device_cfg_read_word(bdf, PCI_SUBSYSTEM_ID, &(pdev->subsystem_device));
    ret = pcie_device_cfg_read_word(bdf, PCI_CLASS_REVISION, &class_revision);
    pdev->class = class_revision & 0xff;
    pdev->revision = class_revision >> 8;

    return pdev;
}

// pci_dev 加入到 g_pcie_device_list_head中， 后续用于统一管理？
int pci_dev_add(struct pci_dev *pdev)
{
    list_add_tail(&(pdev->links), &g_pcie_driver_list_head);
    return OS_OK;
}

#if 0
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
#endif