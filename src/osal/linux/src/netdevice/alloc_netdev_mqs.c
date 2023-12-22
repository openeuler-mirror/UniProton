#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <string.h>
#include "prt_typedef.h"
#include "prt_mem.h"
#include "securec.h"

/**
 * alloc_netdev_mqs - allocate network device
 * @sizeof_priv: size of private data to allocate space for
 * @name: device name format string
 * @name_assign_type: origin of device name
 * @setup: callback to initialize device
 * @txqs: the number of TX subqueues to allocate
 * @rxqs: the number of RX subqueues to allocate
 *
 * Allocates a struct net_device with private data area for driver use
 * and performs basic initialization.  Also allocates subqueue structs
 * for each queue on the device.
 */


struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name,
        unsigned char name_assign_type,
        void (*setup)(struct net_device *),
        unsigned int txqs, unsigned int rxqs)
{
    struct net_device *dev;
    unsigned int alloc_size;
    alloc_size = sizeof(struct net_device);
    struct net_device *p;

    if (sizeof_priv) {
        /* ensure 32-byte alignment of private area */
        alloc_size = (unsigned int)ALIGN(alloc_size, NETDEV_ALIGN);
        alloc_size += sizeof_priv;
    }
    /* ensure 32-byte alignment of whole construct */
    alloc_size += NETDEV_ALIGN - 1;

    p = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, alloc_size);
    if (!p) {
        return NULL;
    }
    memset_s(p, alloc_size, 0, alloc_size);

    dev = (struct net_device *)ALIGN(p, NETDEV_ALIGN);
    dev->padded = (char *)dev - (char *)p;

    setup(dev);
    strcpy(dev->name, name);
    dev->name_assign_type = name_assign_type;
    dev->dev_addr = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, ETH_ALEN);
    if (!dev->dev_addr) {
        (void)PRT_MemFree(OS_MID_APP, p);
        return NULL;
    }
    memset_s(dev->dev_addr, ETH_ALEN, 0, ETH_ALEN);
    return dev;
}