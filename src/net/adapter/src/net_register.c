#include <stdio.h>
#include <stdlib.h>
#include "prt_typedef.h"
#include "arch/net_register.h"

struct ethernet_api g_eth_api;
int ethernetif_api_register(struct ethernet_api *api)
{
    if (api == NULL) {
        return OS_ERROR;
    }

    memcpy(&g_eth_api, api, sizeof(struct ethernet_api));

    return OS_OK;
}