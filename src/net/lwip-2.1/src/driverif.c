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

#include <lwip/sys.h>
#include <lwip/netif.h>
#include <lwip/snmp.h>
#include <lwip/etharp.h>
#include <lwip/sockets.h>
#include <lwip/ethip6.h>

#define LWIP_NETIF_HOSTNAME_DEFAULT         "default"
#define LINK_SPEED_OF_YOUR_NETIF_IN_BPS     100000000 // 100Mbps

#define link_rx_drop cachehit
#define link_rx_overrun cachehit

#define NETIF_NAME_PREFIX_MAX_LENGTH 10
#define NETIF_NAME_PREFIX_ETH "eth"
#define NETIF_NAME_PREFIX_WIFI "wlan"
#define NETIF_NAME_PREFIX_BT "bt"

#ifndef LWIP_NETIF_IFINDEX_MAX_EX
#define LWIP_NETIF_IFINDEX_MAX_EX 255
#endif

static void OsDriverifGetIfnamePrefix(struct netif *netif, char *prefix, int prefixLen)
{
    if (prefix == NULL || netif == NULL) {
        LWIP_ASSERT("invalid param", 0);
        return;
    }
    switch (netif->link_layer_type) {
        case ETHERNET_DRIVER_IF:
            strcpy_s(prefix, prefixLen, NETIF_NAME_PREFIX_ETH);
            break;
        case WIFI_DRIVER_IF:
            strcpy_s(prefix, prefixLen, NETIF_NAME_PREFIX_WIFI);
            break;
        case BT_PROXY_IF:
            strcpy_s(prefix, prefixLen, NETIF_NAME_PREFIX_BT);
            break;
        default:
            LWIP_ASSERT("invalid link_layer_type", 0);
            break;
    }
}

static void OsDriverifInitIfname(struct netif *netif)
{
    struct netif *tmpNetif = NULL;
    char prefix[NETIF_NAME_PREFIX_MAX_LENGTH] = {0};

    OsDriverifGetIfnamePrefix(netif, prefix, NETIF_NAME_PREFIX_MAX_LENGTH);
    netif->name[0] = prefix[0];
    netif->name[1] = prefix[1];

    if (netif->full_name[0] != '\0') {
        LWIP_DEBUGF(DRIVERIF_DEBUG, ("netif already has fullname %s\n", netif->full_name));
        return;
    }
    for (int i = 0; i < LWIP_NETIF_IFINDEX_MAX_EX; ++i) {
        if (snprintf_s(netif->full_name, sizeof(netif->full_name), sizeof(netif->full_name) - 1,
            "%s%d", prefix, i) < 0) {
            break;
        }
        NETIF_FOREACH(tmpNetif) {
            if (strcmp(tmpNetif->full_name, netif->full_name) == 0) {
                break;
            }
        }
        if (tmpNetif == NULL) {
            LWIP_DEBUGF(DRIVERIF_DEBUG, ("set fullname success %s\n", netif->full_name));
            return;
        }
    }
    netif->full_name[0] = '\0';
}

/*
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this driverif
 * @param p the MAC packet to send (e.g. IP packet including MAC_addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t OsDriverifOutput(struct netif *netif, struct pbuf *p)
{
    LWIP_DEBUGF(DRIVERIF_DEBUG, ("OsDriverifOutput : send packet pbuf 0x%p of length %"U16_F" through netif 0x%p\n", \
        (void *)p, p->tot_len, (void *)netif));

#if PF_PKT_SUPPORT
    if (all_pkt_raw_pcbs != NULL) {
        p->flags = (u16_t)(p->flags & ~(PBUF_FLAG_LLMCAST | PBUF_FLAG_LLBCAST | PBUF_FLAG_HOST));
        p->flags |= PBUF_FLAG_OUTGOING;
        (void)raw_pkt_input(p, netif, NULL);
    }
#endif

#if ETH_PAD_SIZE
    (void)pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    netif->drv_send(netif, p);

#if ETH_PAD_SIZE
    (void)pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

static void OsDriverifInputProc(struct netif *netif, struct pbuf *p)
{
    err_t ret = ERR_VAL;
    struct eth_hdr * ethhdr = (struct eth_hdr *)p->payload;
    U16 ethhdrType = ntohs(ethhdr->type);

    switch (ethhdrType) {
        /* IP or ARP packet? */
        case ETHTYPE_IP:
        case ETHTYPE_IPV6:
        case ETHTYPE_ARP:
#if ETHARP_SUPPORT_VLAN
        case ETHTYPE_VLAN:
#endif /* ETHARP_SUPPORT_VLAN */
            LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input : received packet of type %"U16_F"\n", ethhdrType));
            /* full packet send to tcpip_thread to process */
            if (netif->input != NULL) {
                ret = netif->input(p, netif);
            }

            if (ret != ERR_OK) {
                LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input: IP input error\n"));
                (void)pbuf_free(p);
                LINK_STATS_INC(link.drop);
                LINK_STATS_INC(link.link_rx_drop);
                if (ret == ERR_MEM) {
                    MIB2_STATS_NETIF_INC(netif, ifinoverruns);
                    LINK_STATS_INC(link.link_rx_overrun);
                }
            } else {
                LINK_STATS_INC(link.recv);
            }
            break;

        default:
            LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input : received packet is of unsupported type %"U16_F"\n", \
                ethhdrType));
            (void)pbuf_free(p);
            LINK_STATS_INC(link.drop);
            LINK_STATS_INC(link.link_rx_drop);
            break;
    }
}

/*
 * This function should be called by network driver to pass the input packet to LwIP.
 * Before calling this API, driver has to keep the packet in pbuf structure. Driver has to
 * call pbuf_alloc() with type as PBUF_RAM to create pbuf structure. Then driver
 * has to pass the pbuf structure to this API. This will add the pbuf into the TCPIP thread.
 * Once this packet is processed by TCPIP thread, pbuf will be freed. Driver is not required to
 * free the pbuf.
 *
 * @param netif the lwip network interface structure for this driverif
 * @param p packet in pbuf structure format
 */
void driverif_input(struct netif *netif, struct pbuf *p)
{
#if PF_PKT_SUPPORT
#if  (DRIVERIF_DEBUG & LWIP_DBG_OFF)
    U16 ethhdrType;
    struct eth_hdr* ethhdr = NULL;
#endif
    err_t ret = ERR_VAL;
#endif

    LWIP_ERROR("driverif_input : invalid arguments", ((netif != NULL) && (p != NULL)), return);

    LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input : going to receive input packet. netif 0x%p, pbuf 0x%p, \
        packet_length %"U16_F"\n", (void *)netif, (void *)p, p->tot_len));

    /* points to packet payload, which starts with an Ethernet header */
    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (p->len < SIZEOF_ETH_HDR) {
        (void)pbuf_free(p);
        LINK_STATS_INC(link.drop);
        LINK_STATS_INC(link.link_rx_drop);
        return;
    }

#if PF_PKT_SUPPORT
#if  (DRIVERIF_DEBUG & LWIP_DBG_OFF)
    ethhdr = (struct eth_hdr *)p->payload;
    ethhdrType = ntohs(ethhdr->type);
    LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input : received packet of type %"U16_F" netif->input=%p\n", \
        ethhdrType, netif->input));
#endif

    /* full packet send to tcpip_thread to process */
    if (netif->input) {
        ret = netif->input(p, netif);
    }
    if (ret != ERR_OK) {
        LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input: IP input error\n"));
        (void)pbuf_free(p);
        LINK_STATS_INC(link.drop);
        LINK_STATS_INC(link.link_rx_drop);
        if (ret == ERR_MEM) {
            LINK_STATS_INC(link.link_rx_overrun);
        }
    } else {
        LINK_STATS_INC(link.recv);
    }

#else
    OsDriverifInputProc(netif, p);
#endif

    LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_input : received packet is processed\n"));
}

/*
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this driverif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM on Allocation Failure
 *         any other err_t on error
 */
err_t driverif_init(struct netif *netif)
{
    U16 linkLayerType;

    if (netif == NULL) {
        return ERR_IF;
    }
    linkLayerType = netif->link_layer_type;
    LWIP_ERROR("driverif_init : invalid link_layer_type in netif", \
        ((linkLayerType == ETHERNET_DRIVER_IF) \
        || (linkLayerType == WIFI_DRIVER_IF \
        || linkLayerType == BT_PROXY_IF)), \
            return ERR_IF);

    LWIP_ERROR("driverif_init : netif hardware length is greater than maximum supported", \
    (netif->hwaddr_len <= NETIF_MAX_HWADDR_LEN), return ERR_IF);

    LWIP_ERROR("driverif_init : drv_send is null", (netif->drv_send != NULL), return ERR_IF);

#if LWIP_NETIF_PROMISC
    LWIP_ERROR("driverif_init : drv_config is null", (netif->drv_config != NULL), return ERR_IF);
#endif

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = LWIP_NETIF_HOSTNAME_DEFAULT;
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->output = etharp_output;
    netif->linkoutput = OsDriverifOutput;

    /* init the netif's full name */
    OsDriverifInitIfname(netif);

    /* maximum transfer unit */
    netif->mtu = IP_FRAG_MAX_MTU;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
#if DRIVER_STATUS_CHECK
        NETIF_FLAG_DRIVER_RDY |
#endif
#if LWIP_IGMP
        NETIF_FLAG_IGMP |
#endif

        /**
        @page RFC-2710 RFC-2710
        @par Compliant Sections
        Section 5. Node State Transition Diagram
        @par Behavior Description
        MLD messages are sent for multicast addresses whose scope is 2
        (link-local), including Solicited-Node multicast addresses.\n
        Behavior:Stack will send MLD6 report /Done to solicited node multicast address
        if the LWIP_MLD6_ENABLE_MLD_ON_DAD is enabled. By default, this is disabled.
        */
        /* Enable sending MLD report /done for solicited address during neighbour discovery */
#if LWIP_IPV6 && LWIP_IPV6_MLD
#if LWIP_MLD6_ENABLE_MLD_ON_DAD
        NETIF_FLAG_MLD6 |
#endif /* LWIP_MLD6_ENABLE_MLD_ON_DAD */
#endif
        NETIF_FLAG_LINK_UP;

#if DRIVER_STATUS_CHECK
    netif->waketime = -1;
#endif /* DRIVER_STATUS_CHECK */
    LWIP_DEBUGF(DRIVERIF_DEBUG, ("driverif_init : Initialized netif 0x%p\n", (void *)netif));
    return ERR_OK;
}
