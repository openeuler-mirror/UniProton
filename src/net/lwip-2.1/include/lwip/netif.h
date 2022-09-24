/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-09-21
 * Description: 网络
 */

#ifndef LWIP_PORTING_NETIF_H
#define LWIP_PORTING_NETIF_H

#include "lwip/arch.h"
#include <net/if.h>
#include <netinet/ip.h>

#define netif_find OsNetifapiNetifFindByName

#if LWIP_DHCPS
#define LWIP_NETIF_CLIENT_DATA_INDEX_DHCP   LWIP_NETIF_CLIENT_DATA_INDEX_DHCP, \
                                            LWIP_NETIF_CLIENT_DATA_INDEX_DHCPS
#endif

#define linkoutput      linkoutput; \
                        void (*drv_send)(struct netif *netif, struct pbuf *p); \
                        u8_t (*drv_set_hwaddr)(struct netif *netif, u8_t *addr, u8_t len); \
                        void (*drv_config)(struct netif *netif, u32_t config_flags, u8_t setBit); \
                        char full_name[IFNAMSIZ]; \
                        u16_t link_layer_type
#include_next <lwip/netif.h>
#undef linkoutput
#if LWIP_DHCPS
#undef LWIP_NETIF_CLIENT_DATA_INDEX_DHCP
#endif

#include <lwip/etharp.h>

// redefine NETIF_NAMESIZE which was defined in netif.h
#undef NETIF_NAMESIZE
#define NETIF_NAMESIZE IFNAMSIZ

#define LOOPBACK_IF         0
#define ETHERNET_DRIVER_IF  1
#define WIFI_DRIVER_IF      801
#define BT_PROXY_IF         802

err_t driverif_init(struct netif *netif);
void driverif_input(struct netif *netif, struct pbuf *p);

#define netif_get_name(netif) ((netif)->full_name)

#endif /* LWIP_PORTING_NETIF_H */
