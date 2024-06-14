#include <linux/netdevice.h>
#include "i210.h"
#include "prt_typedef.h"

bool netif_carrier_ok(const struct net_device *dev)
{
    return true;
}