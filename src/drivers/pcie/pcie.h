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
 * Description: PCIE功能
 */

#ifndef _PCIE_H_
#define _PCIE_H_

#include "prt_typedef.h"

#define PCIE_DBG_LOG

#ifdef PCIE_DBG_LOG
#define PCIE_DBG_PRINTF(...) printf(__VA_ARGS__)
#else
#define PCIE_DBG_PRINTF(...)
#endif

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void __list_add(struct list_head *_new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
	__list_add(_new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (struct list_head*)LIST_POISON1;
	entry->prev = (struct list_head*)LIST_POISON2;
}

typedef U64 dma_addr_t;
typedef U64 phys_addr_t;
typedef U32 gfp_t;

struct pci_driver;

#define PCI_BDF(b, d, f) ((((b) & 0xff) << 8) | (((d) & 0x1f) << 3) | ((f) & 0x7))
#define PCI_BUS(bdf) (((bdf) >> 8) & 0xff)
#define PCI_DEVFN_FROM_BDF(bdf) ((bdf) & 0xff)

#define PCI_DEVFN(slot, func)   ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn) (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn) ((devfn) & 0x07)

#define	PCI_ANY_ID              ((uint16_t)0xffff)
#define	PCI_VENDOR_ID_INTEL     0x8086
#define	PCI_VENDOR_ID_HUAWEI    0x19e5

#define	PCI_DEVICE(_vendor, _device)                        \
        .vendor = (_vendor), .device = (_device),           \
        .subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

/* For PCI devices, the region numbers are assigned this way: */
enum {
    /* #0-5: standard PCI resources */
    PCI_STD_RESOURCES,
    PCI_STD_RESOURCE_END = 5,

    /* Total resources associated with a PCI device */
    PCI_NUM_RESOURCES,
    /* Preserve this for compatibility */
    DEVICE_COUNT_RESOURCE = PCI_NUM_RESOURCES,
};

struct resource {
    uint64_t start;
    uint64_t end;
    uint32_t flags;
};

#define PCI_IRQ_LEGACY  (1 << 0) /* Allow legacy interrupts */
#define PCI_IRQ_MSI     (1 << 1) /* Allow MSI interrupts */
#define PCI_IRQ_MSIX    (1 << 2) /* Allow MSI-X interrupts */
#define PCI_IRQ_MAX_NUM 8

/* pci设备结构体 */
struct pci_dev {
    struct list_head links;
    struct pci_driver *pdrv;
    uint16_t device;
    uint16_t vendor;
    uint16_t subsystem_vendor;
    uint16_t subsystem_device;
    unsigned int irq[PCI_IRQ_MAX_NUM];
    unsigned int bdf;
    unsigned int bus_no;
    unsigned int devfn;
    uint32_t class;
    uint8_t revision;
    bool msi_enabled;
    struct resource resource[DEVICE_COUNT_RESOURCE]; /* I/O and memory regions */
    dma_addr_t dma_iova;
    phys_addr_t dma_pa;
    size_t dma_size;
    size_t dma_used_size;
};

struct pci_device_id {
    uint16_t vendor;
    uint16_t device;
    uint16_t subvendor;
    uint16_t subdevice;
    uint32_t class;
    uint32_t class_mask;
    uintptr_t driver_data;
};

/* pci 驱动结构体 */
struct pci_driver {
    struct list_head links;
    char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
};

int pci_frame_init(uint64_t pci_cfg_base);
int pci_register_driver(struct pci_driver *pci_drv);

int pci_alloc_irq_vectors(struct pci_dev *dev, unsigned int min_vecs,
    unsigned int max_vecs, unsigned int flags);
int pci_irq_vector(struct pci_dev *dev, int i);

int pci_enable_device(struct pci_dev *dev);
int pci_disable_device(struct pci_dev *dev);
void pci_set_master(struct pci_dev *dev);
int pci_request_regions(struct pci_dev *pdev, const char *res_name);
void pci_release_regions(struct pci_dev *pdev);

void *dma_alloc_coherent(struct pci_dev *dev, size_t size, dma_addr_t *dma_handle, gfp_t gfp);
void dma_free_coherent(struct pci_dev *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle);

#define pci_resource_start(dev, bar)    ((dev)->resource[(bar)].start)
#define pci_resource_end(dev, bar)      ((dev)->resource[(bar)].end)
#define pci_resource_flags(dev, bar)    ((dev)->resource[(bar)].flags)
#define pci_resource_len(dev, bar) ((pci_resource_end((dev), (bar)) == 0) ? \
    0 : (pci_resource_end((dev), (bar)) - pci_resource_start((dev), (bar)) + 1))

#endif