#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/msi.h>
#include <linux/syscalls.h>
#include <linux/umh.h>
#include "hpm_node.h"

#define MMU_DMA_ADDR    0x202780100000ULL
#define MMU_DMA_LENGTH  0xA00000

const char g_hpm_driver_name[] = "huawei_pci_model";

static struct pci_device_id hpm_pci_tbl[] = {
    { PCI_VDEVICE(HUAWEI, 0xa221) },
    { 0, } /* required last entry */
};

MODULE_DEVICE_TABLE(pci, hpm_pci_tbl);

typedef struct hpm_s {
    struct pci_dev *pdev;
    unsigned int nvec;
    void __iomem *bar0;
    unsigned long bar0_len;
    void __iomem *bar2;
    unsigned long bar2_len;
    phys_addr_t dma_pa; /* dma 物理地址 */
    dma_addr_t  dma_iova; /* dma io虚拟地址 */
    void *      dma_va; /* dma 虚拟地址 */
    size_t      dma_size;
} hpm_t;

static irqreturn_t irq_handler_func(int irq, void *irq_instance)
{
    hpm_t *hpm_data = (hpm_t *)irq_instance;
    struct pci_dev *pdev = hpm_data->pdev;

    printk(KERN_INFO "irq_handler_func irq:%d handled!! But %s intrrupt should to rtos-cpu:%lu!\n",
        irq, pci_name(pdev), intr_bind_cpu);

    return IRQ_HANDLED;
}

static int irq_alloc_config_test(struct pci_dev *pdev)
{
    int nvec = 4;
    int nvec_min = 2;
    int ret;
    unsigned int i;
    hpm_t *data = pci_get_drvdata(pdev);

    nvec = pci_alloc_irq_vectors(pdev, nvec_min, nvec, PCI_IRQ_MSIX);
    if (nvec < 0) {
        printk(KERN_INFO "pci_alloc_irq_vectors failed, ret:0x%x!", nvec);
        return nvec;
    }
    for (i = 0; i < nvec; i++) {
        ret = pci_request_irq(pdev, i, irq_handler_func, NULL, (void *)data, "hpm_demo_irq%u", i);
        if (ret) {
            printk(KERN_INFO "pci_request_irq failed, ret:0x%x!", ret);
        }
    }
    return nvec;
}

unsigned int g_hpm_num = 0;
hpm_t g_hpm_data[2];

static int hpm_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int err;
    u8 __iomem *bar_addr;
    u32 bar_size;
    void __iomem *bar;
    int i;
    int irq_num;
    const char *eth_dev_name = pci_name(to_pci_dev(&pdev->dev));
    printk(KERN_INFO "%s probe func: eth_dev_name:%s\n", g_hpm_driver_name,
        eth_dev_name);

    err = pci_enable_device(pdev);
    if (err != 0) {
        printk(KERN_INFO "pci enable device failed\n");
        return err;
    }

    err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
    if (err != 0) {
        err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
        if (err) {
            dev_err(&pdev->dev, "No usable DMA configuration, aborting\n");
            goto err_dma;
        }
    }

    err = pci_request_regions(pdev, g_hpm_driver_name);
    if (err)
        goto err_dma;

    pci_set_master(pdev);
    pci_save_state(pdev);

    if (g_hpm_num >= 2) {
        goto err_dma;
    }

    pci_set_drvdata(pdev, &(g_hpm_data[g_hpm_num]));

    /* 打印设备的BAR地址和大小 */
    bar_addr = (u8 __iomem *)pci_resource_start(pdev, 0);
    bar_size = pci_resource_len(pdev, 0);
    printk(KERN_INFO "hpm: BAR0 address = 0x%llx, size = %u", (u64)bar_addr,
        bar_size);

    /* 映射BAR空间 */
    bar = pci_iomap(pdev, 0, bar_size);
    if (!bar) {
        printk(KERN_INFO "hpm: Failed to map BAR2");
        err = -ENOMEM;
        goto err_pci_reg;
    }

    g_hpm_data[g_hpm_num].bar0 = bar;
    g_hpm_data[g_hpm_num].bar0_len = bar_size;

    /* 打印设备的BAR地址和大小 */
    bar_addr = (u8 __iomem *)pci_resource_start(pdev, 2);
    bar_size = pci_resource_len(pdev, 2);
    printk(KERN_INFO "hpm: BAR2 address = 0x%llx, size = %u", (u64)bar_addr,
        bar_size);

    /* 映射BAR空间 */
    bar = pci_iomap(pdev, 2, bar_size);
    if (!bar) {
        printk(KERN_INFO "hpm: Failed to map BAR2");
        err = -ENOMEM;
        goto err_pci_reg;
    }

    g_hpm_data[g_hpm_num].bar2 = bar;
    g_hpm_data[g_hpm_num].bar2_len = bar_size;

    bar = g_hpm_data[g_hpm_num].bar0;
    printk(KERN_INFO "resource0 dump:");
    for (i = 0; i < g_hpm_data[g_hpm_num].bar0_len && i < 0x20; i += 4) {
        printk(KERN_INFO "%08x ", *((unsigned int*)(bar + i)));
    }

    bar = g_hpm_data[g_hpm_num].bar2;
    printk(KERN_INFO "resource2 dump:");
    for (i = 0; i < g_hpm_data[g_hpm_num].bar2_len && i < 0x20; i += 4) {
        printk(KERN_INFO "%08x ", *((unsigned int*)(bar + i)));
    }

    irq_num = irq_alloc_config_test(pdev);
    if (irq_num <= 0) {
        printk(KERN_INFO "irq_alloc_config_test failed:0x%x\n", irq_num);
        goto out_err;
    }

    /* 对预留的物理内存MMU_DMA_ADDR,
       进行smmu映射得到dma_iova, 配置给dev的dma寄存器, 供设备读写
       进行mmu映射得到dma_va, 供软件/CPU访问
    */
    g_hpm_data[g_hpm_num].dma_size = MMU_DMA_LENGTH;
    g_hpm_data[g_hpm_num].dma_pa = MMU_DMA_ADDR;
    g_hpm_data[g_hpm_num].dma_iova = dma_map_resource(&pdev->dev,
        g_hpm_data[g_hpm_num].dma_pa, g_hpm_data[g_hpm_num].dma_size,
        DMA_BIDIRECTIONAL, 0);
    g_hpm_data[g_hpm_num].dma_va = memremap(g_hpm_data[g_hpm_num].dma_pa,
        g_hpm_data[g_hpm_num].dma_size, MEMREMAP_WT);

    g_hpm_data[g_hpm_num].pdev = pdev;
    g_hpm_data[g_hpm_num].nvec = irq_num;

    /* 额外增加 */
    pci_var_node_create(pdev, intr_bind_cpu, irq_num,
        g_hpm_data[g_hpm_num].dma_pa, g_hpm_data[g_hpm_num].dma_iova,
        g_hpm_data[g_hpm_num].dma_size);
    printk(KERN_INFO "~~~~~~~~~~~pci_var_node_create~~~~~~~~~~~\n");

    g_hpm_num++;
    return 0;

out_err:
err_pci_reg:
    pci_release_regions(pdev);
err_dma:
    pci_disable_device(pdev);
    return err;
}

static void hpm_remove(struct pci_dev *pdev)
{
    uint32_t i;
    hpm_t *hpm_data;

    /* 额外增加 */
    pci_var_node_destroy(pdev);
    printk(KERN_INFO "~~~~~~~~~~~pci_var_node_destroy~~~~~~~~~~~\n");

    hpm_data = (hpm_t *)pci_get_drvdata(pdev);
    if (hpm_data != NULL) {
        pci_iounmap(pdev, hpm_data->bar0);
        pci_iounmap(pdev, hpm_data->bar2);
        for (i = 0; i < hpm_data->nvec; i++) {
            pci_free_irq(pdev, i, (void*)hpm_data);
        }
    }

    dma_unmap_resource(&(pdev->dev), hpm_data->dma_iova, hpm_data->dma_size,
        DMA_BIDIRECTIONAL, 0);
    pci_free_irq_vectors(pdev);

    pci_release_regions(pdev);
    pci_disable_device(pdev);
}

static struct pci_driver hpm_driver = {
    .name = g_hpm_driver_name,
    .id_table = hpm_pci_tbl,
    .probe = hpm_probe,
    .remove = hpm_remove,
};

static int __init hpm_init(void)
{
    printk(KERN_INFO "Hello, hpm_init!\n");

    return pci_register_driver(&hpm_driver);
}

static void __exit hpm_exit(void)
{
    pci_unregister_driver(&hpm_driver);
    printk(KERN_INFO "Byebye, hpm_exit!");
}

module_init(hpm_init);
module_exit(hpm_exit);

MODULE_AUTHOR("OpenEuler Embedded");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("HPM PCIe Module driver");