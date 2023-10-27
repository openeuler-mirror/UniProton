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
#include "prt_hwi.h"

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

void irq_handler(uintptr_t arg)
{
    int irq = *((int*)arg);
    printf("irq_handler:%d\n", irq);
}

#define HCLGE_PF_OTHER_INT_REG 0x20600

static int hpm_probe_flag = 0;
int hpm_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int32_t ret;

    /* demo代码仅找一个设备验证，匹配到多个设备也不执行多次probe */
    if (dev->bus_no != 0xbd) {
        return 0;
    }
    if (hpm_probe_flag > 0) {
        return 0;
    }
    hpm_probe_flag++;

    printf("hpm_probe bdf:%02x:%02x.%01x\r\n", dev->bus_no,
        PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

    ret = pci_enable_device(dev);
    if (ret != 0) {
        printf("func:%s line:%d, ret:%x\r\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = pci_request_regions(dev, NULL);
    if (ret != 0) {
        printf("func:%s line:%d, ret:%x\r\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    pci_set_master(dev);

#if 0
    int irq_num = pci_alloc_irq_vectors(dev, 2, 4, PCI_IRQ_MSIX);
    if (irq_num < 0) {
        return -1;
    }

    for (int i= 0; i < irq_num; i++) {
        int irq = pci_irq_vector(dev, i);
        printf("irq%d:%d\r\n", i, irq);

        ret = PRT_HwiSetAttr(irq, 12, OS_HWI_MODE_ENGROSS);
        if (ret != OS_OK) {
            printf("irq%d: %d setAttr failed, ret:%x\r\n", i, irq, ret);
        }

        intptr_t hwiarg = &(dev->irq[i]);
        ret = PRT_HwiCreate(irq, irq_handler, hwiarg);
        if (ret != OS_OK) {
            printf("irq%d: %d create failed, ret:%x\r\n", i, irq, ret);
        }
    }
#endif

    mmu_info_dump();

#if 1
    uintptr_t bar0 = pci_resource_start(dev, 0);
    uint32_t bar0_size = pci_resource_len(dev, 0);
    uint32_t bar0_flags = pci_resource_flags(dev, 0);
    printf("bar0:%llx size:%u flag:%x\n", bar0, bar0_size, bar0_flags);

    uintptr_t bar2 = pci_resource_start(dev, 2);
    uint32_t bar2_size = pci_resource_len(dev, 2);
    uint32_t bar2_flags = pci_resource_flags(dev, 2);
    printf("bar2:%llx size:%u flag:%x\n", bar2, bar2_size, bar2_flags);

    ret = mmu_request(bar0, bar0_size);
    if (ret != 0) {
        return ret;
    }

    ret = mmu_request(bar2, bar2_size);
    if (ret != 0) {
        return ret;
    }

    ret = mmu_update();
    if (ret != 0) {
        return ret;
    }

    printf("resource0 dump:\r\n");
    for (int i = 0; i < bar0_size && i < 0x20; i += 4) {
        printf("%08x\r\n", *((unsigned int*)(bar0 + i)));
    }

    printf("resource0 dump:\r\n");
    for (int i = 0; i < bar2_size && i < 0x20; i += 4) {
        printf("%08x\r\n", *((unsigned int*)(bar2 + i)));
    }
#endif

#if 0
    struct resource *dev_rs;
    uintptr_t bar, bar_size;

    dev_rs = dev->resource;
    printf("resource[%u]: [%llx %llx] %x\r\n", 0, dev_rs[0].start, dev_rs[0].end, dev_rs[0].flags);
    printf("resource[%u]: [%llx %llx] %x\r\n", 2, dev_rs[2].start, dev_rs[2].end, dev_rs[2].flags);

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

    bar = dev_rs[2].start;
    bar_size = dev_rs[2].end - dev_rs[2].start + 1;
    ret = mmu_request(bar, bar_size);
    if (ret != 0) {
        return ret;
    }

    ret = mmu_update();
    if (ret != 0) {
        return ret;
    }

    bar = dev_rs[0].start;
    printf("resource0 dump:\r\n");
    for (int i = 0; i < (dev_rs[0].end - dev_rs[0].start + 1) && i < 0x20; i += 4) {
        printf("%08x\r\n", *((uint32_t *)(bar + i)));
    }

    bar = dev_rs[2].start;
    printf("resource2 reg test:\r\n");

    intptr_t *reg_addr = bar + HCLGE_PF_OTHER_INT_REG;
    int32_t reg_value = *reg_addr;
    *reg_addr = (reg_value & 0x1) ? (reg_value & (~0x1)) : (reg_value | 0x1);
    printf("0x%08x == > 0x%08x", reg_value, *reg_addr);
#endif

#if 1
    int irq_num = pci_alloc_irq_vectors(dev, 2, 4, PCI_IRQ_MSIX);
    if (irq_num < 0) {
        return -1;
    }

    for (int i= 0; i < irq_num; i++) {
        int irq = pci_irq_vector(dev, i);
        printf("irq%d:%d\r\n", i, irq);

        ret = PRT_HwiSetAttr(irq, 12, OS_HWI_MODE_ENGROSS);
        if (ret != OS_OK) {
            printf("irq%d: %d setAttr failed, ret:%x\r\n", i, irq, ret);
        }

        intptr_t hwiarg = &(dev->irq[i]);
        ret = PRT_HwiCreate(irq, irq_handler, hwiarg);
        if (ret != OS_OK) {
            printf("irq%d: %d create failed, ret:%x\r\n", i, irq, ret);
        }
    }
#endif

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
    ret = pci_register_driver(&hpm_driver);
    if (ret != ok) {
        return;
    }

    return;
}
