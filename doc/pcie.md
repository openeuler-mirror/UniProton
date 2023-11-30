# Uniproton PCIe使用指南

## 1 整体方案：
当前pcie支持是基于混合部署场景的，即需要在linux侧完成部分非实时功能，并将结果通过文件的形式预备好，这样uniproton的pcie在初始化过程中，可以在linux侧获取这些预留信息。

## 2 编译时使能pcie功能：
defconfig 设置CONFIG_OS_OPTION_PCIE=y。

## 3 添加自己的pcie设备驱动：
参考 pcie_demo 文件(UniProton/demos/kp920/apps/openamp/pcie_demo.c)，支持pcie驱动如下接口：

### 3.1 配置空间初始化
* ```pci_frame_init```注册 pcie 的配置空间的基地址，驱动框架将基于该基地址扫描256个bus，以匹配所要驱动的pcie设备。

### 3.2 驱动结构体
* ```struct pci_driver```用于传递pci设备的id_table, 挂接probe函数等功能。
```
struct pci_driver {
    struct list_head links;
    char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
};
```

### 3.2 驱动接口注册
* ```pci_register_driver```将驱动注册到pcie框架中。

### 3.2 设备的探测
* ```probe```函数，如果能匹配到设备，则执行驱动中的probe函数。

### 3.3 Bar地址
* ```pci_resource_start```、```pci_resource_len```、```pci_resource_flags``` 三个宏分别用于获取pcie设备的bar空间起始地址，大小以及类型，然后通过```mmu_request```和```mmu_update```完成地址映射之后，就可以访问bar空间了。

### 3.3 申请中断号
* ```pci_alloc_irq_vectors```用于申请多个中断号，并支持MSI/MSIX类型中断。

### 3.4 注册中断服务
* ```pci_irq_vector```获取中断号之后，通过```PRT_HwiSetAttr```和```PRT_HwiCreate```注册中断服务。

### 3.5 申请dma空间
* ```dma_alloc_coherent```用于申请dma空间。

## 4 linux侧进行预配置

linux 侧需要进行常规的pcie驱动注册，中断的申请，dma空间申请，然后通过调用函数```pci_var_node_create```，创建文件节点，记录申请的中断和申请的dma空间。参考hpm_demo文件 (UniProton/demos/kp920/apps/pcie_demo_ko/hpm_demo.c)。需要注意如下几点：

### 4.1 DMA空间预留和申请

dma空间申请需要使用预留的物理地址，通过函数```dma_map_resource```完成申请，这样uniproton侧才能同时访问该地址。

### 4.2 中断亲和性配置

默认申请的中断会自动分配到所有的核，```pci_var_node_create```中会将中断亲和性设置到uniproton核(通过intr_bind_cpu设置)，依赖linux函数```irq_force_affinity```，如果低版本的内核(5.14以下)需要想其他办法(kallsyms、kprobe)获取到irq_force_affinity函数调用权限。
如果开启了irqbalance服务，需要关闭或配置为不更改指定中断的亲和性，参考如下命令：

```sh
systemctl disable irqbalance    #永久关闭irqbalance服务
systemctl stop irqbalance       #关闭irqbalance服务
systemctl status irqbalance     #查看irqbalance服务

irqbalance --banirq=44          #禁止自动更改44号irq的中断亲和性
```