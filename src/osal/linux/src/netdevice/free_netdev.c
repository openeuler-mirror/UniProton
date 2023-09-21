#include <linux/netdevice.h>
#include "prt_typedef.h"
#include "prt_mem.h"

void free_netdev(struct net_device *dev)
{
    char *addr = (char *)dev - dev->padded;
    (void)PRT_MemFree(OS_MID_APP, dev->dev_addr);
    (void)PRT_MemFree(OS_MID_APP, addr);
}