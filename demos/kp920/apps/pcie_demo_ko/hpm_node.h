#ifndef _HPM_NODE_H_
#define _HPM_NODE_H_

#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

static unsigned long intr_bind_cpu = 23; /* 设备中断设置到指定cpu上， 默认指定cpu23 */
module_param(intr_bind_cpu, ulong, S_IRUSR);

#define UNIPROTON_NODE_PATH "/run/pci_uniproton/"

static inline int system_call_mkdir(const char* path)
{
    char cmd_path[] = "/bin/mkdir";
    char *cmd_argv[] = { cmd_path, "-p", path, NULL };
    char *cmd_envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret;

    ret = call_usermodehelper(cmd_path, cmd_argv, cmd_envp, UMH_WAIT_PROC);
    if (ret != 0) {
        printk(KERN_INFO "Failed to execute mkdir %s, ret:0x%x\n", path, ret);
        return ret;
    }
    printk(KERN_INFO "Successed to execute mkdir %s!\n", path);
    return ret;
}

static inline int system_call_rmdir(const char* path)
{
    char cmd_path[] = "/usr/bin/rm";
    char *cmd_argv[] = { cmd_path, "-rf", path, NULL };
    char *cmd_envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret;

    ret = call_usermodehelper(cmd_path, cmd_argv, cmd_envp, UMH_WAIT_PROC);
    if (ret != 0) {
        printk(KERN_INFO "Failed to execute mkdir %s, ret:0x%x\n", path, ret);
        return ret;
    }
    printk(KERN_INFO "Successed to execute mkdir %s!\n", path);
    return ret;
}

static inline int system_call_touch(const char* path)
{
    char cmd_path[] = "/bin/touch";
    char *cmd_argv[] = { cmd_path, path, NULL };
    char *cmd_envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret;

    ret = call_usermodehelper(cmd_path, cmd_argv, cmd_envp, UMH_WAIT_PROC);
    if (ret != 0) {
        printk(KERN_INFO "Failed to execute touch %s, ret:0x%x", path, ret);
        return ret;
    }
    printk(KERN_INFO "Successed to execute touch %s!\n", path);
    return ret;
}

static inline unsigned int irq_to_hwirq(unsigned int irq)
{
    struct irq_data *data = irq_get_irq_data(irq);
    unsigned long hwirq;
    while (data) {
        hwirq = data->hwirq;
        data = data->parent_data;
    }
    return hwirq;
}

/* 这里先由用户配置，后续从pdev配置读取 */
static inline int get_pci_interrupt_count(struct pci_dev *pdev)
{
    int count = 1024;
    (void)pdev;
    return count;
}

/* 这里irq_num先由用户配置，后续从pdev配置读取 */
static inline void pci_var_node_create(struct pci_dev *pdev, unsigned long cpu_id, int irq_num)
{
    uint32_t bdf;
    int ret;
    uint32_t i;
    char path_var_dev[256];

    if (pdev == NULL) {
        printk(KERN_INFO, "pci_var_node_create pdev is null!");
        return;
    }

    bdf = (pdev->bus->number << 8) | (pdev->devfn & 0xff);
    sprintf(path_var_dev, "%s%04x", UNIPROTON_NODE_PATH, bdf);
    ret = system_call_mkdir(path_var_dev);
    if (ret != 0) {
        printk(KERN_INFO "Failed to execute mkdir command, ret:0x%x", ret);
        return;
    }

    for (i = 0; i < irq_num; i++) {
        int irq = pci_irq_vector(pdev, i);
        if (irq < 0) {
            break;
        }
        ret = irq_force_affinity(irq, cpumask_of(cpu_id));
        if (ret) {
            printk(KERN_INFO "irq_force_affinity to cpu:%u failed! ret:0x%x", cpu_id, ret);
        }
        uint32_t hwirq = irq_to_hwirq(irq);
        sprintf(path_var_dev, "%s%04x/msi_%u_%u", UNIPROTON_NODE_PATH, bdf, i, hwirq);
        ret = system_call_touch(path_var_dev);
        if (ret != 0) {
            printk(KERN_INFO "Failed to execute touch command, ret:0x%x", ret);
            return;
        }
    }
}

static inline void pci_var_node_destroy(struct pci_dev *pdev)
{
    uint32_t bdf;
    int ret;
    char path_var_dev[256];

    if (pdev == NULL) {
        printk(KERN_INFO, "pci_var_node_destroy pdev is null!");
        return;
    }

    bdf = (pdev->bus->number << 8) | (pdev->devfn & 0xff);
    sprintf(path_var_dev, "%s%04x", UNIPROTON_NODE_PATH, bdf);
    ret = system_call_rmdir(path_var_dev);
    if (ret != 0) {
        printk(KERN_INFO "Failed to execute command, ret:0x%x", ret);
        return;
    }
}

#endif //_HPM_NODE_H_