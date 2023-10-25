/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-10-17
 * Description: PCIE功能demo
 */

#include "pcie.h"

#define PCI_VENDOR_ID PCI_VENDOR_ID_HUAWEI
#define PCI_DEVICE_ID 0xa221 /* HNAE3_DEV_ID */

#define error -1
#define ok 0

static char g_hpm_driver_name[] = "huawei_pci_model";
static struct pci_device_id g_hpm_pci_dev_tbl[] = {
    { PCI_DEVICE(PCI_VENDOR_ID, PCI_DEVICE_ID) },
    { 0, 0 },
};

extern S32 mmu_request(U64 phy_addr, U64 length);
extern void mmu_release(U64 virt_addr);
extern S32 mmu_update(void);

static int hpm_probe_flag = 0;
int hpm_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct resource *dev_rs;
    uintptr_t bar, bar_size;
    int32_t ret, i;

    /* demo代码仅找一个设备验证，匹配到多个设备也不执行多次probe */
    if (dev->bus_no != 0xbd) {
        return 0;
    }
    if (hpm_probe_flag > 0) {
        return 0;
    }
    hpm_probe_flag++;

    printf("hpm_probe bdf:%02x:%02x.%01x\n", dev->bus_no,
        PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

    dev_rs = dev->resource;
    printf("resource[%u]: [%llx %llx] %x", 0, dev_rs[0].start, dev_rs[0].end, dev_rs[0].flags);
    printf("resource[%u]: [%llx %llx] %x", 2, dev_rs[2].start, dev_rs[2].end, dev_rs[2].flags);

    bar = dev_rs[0].start;
    bar_size = dev_rs[0].end - dev_rs[0].start + 1;
    ret = mmu_request(bar, bar_size);
    if (ret != 0) {
        return ret;
    }

    ret = mmu_update();
    if (ret != 0) {
        return ret;
    }

    bar = dev_rs[0].start;
    printf("resource0 dump:\n");
    for (i = 0; i < (dev_rs[0].end - dev_rs[0].start + 1) && i < 0x40; i += 4) {
        ((uint32_t *)bar)[i] = 0xa5a50000 + i;
        printf("%08x%c", ((uint32_t *)bar)[i], (i % 16) ? ' ' : '\n');
    }

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

#ifndef MMU_ECAM_ADDR
#define MMU_ECAM_ADDR 0xd0000000ULL
#endif

void test_pcie(void)
{
    int ret;

    // 由系统初始化调用
    ret = pci_frame_init(MMU_ECAM_ADDR);
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
