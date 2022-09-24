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

#include "lwip/opt.h"

#if LWIP_IFADDRS
#if (LWIP_IPV4 || LWIP_IPV6) && LWIP_SOCKET
#include "ifaddrs.h"

#include <stdlib.h>

#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/priv/sockets_priv.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"

struct TagIfaddrsStorage {
    struct ifaddrs ifa;
    union {
        struct sockaddr sa;
        struct sockaddr_in s4;
#if LWIP_IPV6
        struct sockaddr_in6 s6;
#endif
    } addr, netmask, dstaddr;
    U8 name[IFNAMSIZ];
};

struct TagGetifaddrsArg {
    struct ifaddrs **ifap;
    sys_sem_t cbCompleted;
    S32 ret;
};

static S32 g_tcpipInitFinish = 1;
static void OsLwipFreeifaddrs(struct ifaddrs *ifa);
static void OsIfaddrsAddTail(struct ifaddrs **ifap, struct ifaddrs *ifaddr)
{
    struct ifaddrs *temp = NULL;

    ifaddr->ifa_next = NULL;
    if (*ifap == NULL) {
        *ifap = ifaddr;
        return;
    }

    for (temp = *ifap; temp->ifa_next != NULL; temp = temp->ifa_next) {
        /* nothing */
    }

    temp->ifa_next = ifaddr;
}

static struct TagIfaddrsStorage *OsNewIfaddrsStorage(void)
{
    struct ifaddrs *ifaddr = NULL;
    struct TagIfaddrsStorage *ifStorage = (struct TagIfaddrsStorage *)mem_malloc(sizeof(struct TagIfaddrsStorage));
    if (ifStorage == NULL) {
        return NULL;
    }
    if (memset_s((void *)ifStorage, sizeof(struct TagIfaddrsStorage), 0, sizeof(struct TagIfaddrsStorage)) != EOK) {
        free(ifStorage);
        return NULL;
    }
    ifaddr = &ifStorage->ifa;
    ifaddr->ifa_name = ifStorage->name;
    ifaddr->ifa_addr = &ifStorage->addr.sa;
    ifaddr->ifa_netmask = &ifStorage->netmask.sa;
    ifaddr->ifa_dstaddr = &ifStorage->dstaddr.sa;
    return ifStorage;
}

static int OsGetIfaName(struct netif *netif, struct ifaddrs *ifaddr)
{
    int ret;

    if (netif->link_layer_type == LOOPBACK_IF) {
        ifaddr->ifa_flags |= IFF_LOOPBACK;
        ret = snprintf_s(ifaddr->ifa_name, NETIF_NAMESIZE, (NETIF_NAMESIZE - 1), "%.2s", netif->name);
    } else {
        ret = snprintf_s(ifaddr->ifa_name, NETIF_NAMESIZE, (NETIF_NAMESIZE - 1), "%s", netif_get_name(netif));
    }

    return ret;
}

#if LWIP_IPV4
static int OsGetIpv4Ifaddr(struct netif *netif, struct ifaddrs *ifaddr)
{
    struct sockaddr_in *addrIn = NULL;

    if (netif->flags & NETIF_FLAG_UP) {
        ifaddr->ifa_flags |= IFF_UP;
    }

    if (netif->flags & NETIF_FLAG_ETHARP) {
        ifaddr->ifa_flags = ifaddr->ifa_flags & ((unsigned int)(~IFF_NOARP));
    } else {
        ifaddr->ifa_flags |= IFF_NOARP;
    }

    if (netif->flags & NETIF_FLAG_BROADCAST) {
        ifaddr->ifa_flags |= IFF_BROADCAST;
    }

#if LWIP_DHCP
    if (dhcp_supplied_address(netif)) {
        ifaddr->ifa_flags |= IFF_DYNAMIC;
    }
#endif

#if LWIP_IGMP
    if (netif->flags & NETIF_FLAG_IGMP) {
        ifaddr->ifa_flags |= IFF_MULTICAST;
    }
#endif

    if (netif->flags & NETIF_FLAG_LINK_UP) {
        ifaddr->ifa_flags |= IFF_RUNNING;
    }

#if LWIP_HAVE_LOOPIF
    if (netif->link_layer_type == LOOPBACK_IF) {
        addrIn = (struct sockaddr_in *)ifaddr->ifa_addr;
        addrIn->sin_family = AF_INET;
        addrIn->sin_addr.s_addr = ((ip4_addr_t *)&netif->ip_addr)->addr;
    } else
#endif
    {
        addrIn = (struct sockaddr_in *)ifaddr->ifa_addr;
        addrIn->sin_family = AF_INET;
        addrIn->sin_addr.s_addr = ((ip4_addr_t *)&netif->ip_addr)->addr;

        addrIn = (struct sockaddr_in *)ifaddr->ifa_netmask;
        addrIn->sin_family = AF_INET;
        addrIn->sin_addr.s_addr = ((ip4_addr_t *)&netif->netmask)->addr;

        addrIn = (struct sockaddr_in *)ifaddr->ifa_broadaddr;
        addrIn->sin_family = AF_INET;
        addrIn->sin_addr.s_addr = (((ip4_addr_t *)&netif->ip_addr)->addr & ((ip4_addr_t *)&netif->netmask)->addr) |
                                  ~((ip4_addr_t *)&netif->netmask)->addr;
    }

    return OsGetIfaName(netif, ifaddr);
}
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
/* Stack support to retrieve the below flags for ipv6
IFF_UP
IFF_MULTICAST
IFF_RUNNING
IFF_LOOPBACK
*/
static int OsGetIpv6Ifaddr(struct netif *netif, struct ifaddrs *ifaddr, int tmp_index)
{
    struct sockaddr_in6 *addrIn6 = NULL;

    /* As of now supports the below falgs only */
    if (netif->flags & NETIF_FLAG_UP) {
        ifaddr->ifa_flags |= IFF_UP;
    }

#if LWIP_IPV6_MLD
    if (netif->flags & NETIF_FLAG_MLD6) {
        ifaddr->ifa_flags |= IFF_MULTICAST;
    }
#endif

    if (netif->flags & NETIF_FLAG_LINK_UP) {
        ifaddr->ifa_flags |= IFF_RUNNING;
    }

    addrIn6 = (struct sockaddr_in6 *)ifaddr->ifa_addr;
    addrIn6->sin6_family = AF_INET6;
    inet6_addr_from_ip6addr(&addrIn6->sin6_addr, (ip6_addr_t *)&netif->ip6_addr[tmp_index]);

    return OsGetIfaName(netif, ifaddr);
}
#endif

static void OsGetIfaddrsInternal(struct TagGetifaddrsArg *arg)
{
    struct netif *netif = NULL;
    struct ifaddrs *ifaddr = NULL;
    struct TagIfaddrsStorage *ifStorage = NULL;

#if LWIP_IPV6
    int n;
#endif

    arg->ret = ENOMEM;
    for (netif = netif_list; netif != NULL; netif = netif->next) {
#if LWIP_IPV4
        ifStorage = OsNewIfaddrsStorage();
        if (ifStorage == NULL) {
            /* ifap is assigned to NULL in getifaddrs, so garbage value will not be there */
            OsLwipFreeifaddrs(*(arg->ifap));
            arg->ret = ENOMEM;
#if !LWIP_TCPIP_CORE_LOCKING
            sys_sem_signal(&arg->cbCompleted);
#endif
            return;
        }

        /* if get one or more netif info, then getifaddrs return 0(OK) */
        arg->ret = 0;
        ifaddr = &ifStorage->ifa;
        (void)OsGetIpv4Ifaddr(netif, ifaddr);
        OsIfaddrsAddTail(arg->ifap, ifaddr);
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
        for (n = 0; n < LWIP_IPV6_NUM_ADDRESSES; n++) {
            if ((netif->ip6_addr_state[n] & IP6_ADDR_VALID) == 0) {
                continue;
            }
            ifStorage = OsNewIfaddrsStorage();
            if (ifStorage == NULL) {
                /* ifap is assigned to NULL in getifaddrs, so garbage value will not be there */
                OsLwipFreeifaddrs(*(arg->ifap));
                arg->ret = ENOMEM;
#if !LWIP_TCPIP_CORE_LOCKING
                sys_sem_signal(&arg->cbCompleted);
#endif
                return;
            }

            /* if get one or more netif info, then getifaddrs return 0(OK) */
            arg->ret = 0;
            ifaddr = &ifStorage->ifa;
            (void)OsGetIpv6Ifaddr(netif, ifaddr, n);
            OsIfaddrsAddTail(arg->ifap, ifaddr);
        }
#endif
    }

#if !LWIP_TCPIP_CORE_LOCKING
    sys_sem_signal(&arg->cbCompleted);
#endif
    return;
}

static int OsLwipGetifaddrs(struct ifaddrs **ifap)
{
    struct TagGetifaddrsArg arg;

    LWIP_ERROR("OsLwipGetifaddrs : ifap is NULL", (ifap != NULL), return ERR_ARG);
    *ifap = NULL;

    if (!g_tcpipInitFinish) {
        set_errno(EACCES);
        return -1;
    }
    arg.ret = 0;
    arg.ifap = ifap;

#if LWIP_TCPIP_CORE_LOCKING
    LOCK_TCPIP_CORE();
    OsGetIfaddrsInternal(&arg);
    UNLOCK_TCPIP_CORE();
#else

    if (sys_sem_new(&arg.cbCompleted, 0) != ERR_OK) {
        set_errno(ENOMEM);
        return -1;
    }

    tcpip_callback((tcpip_callback_fn)OsGetIfaddrsInternal, &arg);
    (void)sys_arch_sem_wait(&arg.cbCompleted, 0);
    sys_sem_free(&arg.cbCompleted);
#endif

    if (arg.ret != 0) {
        set_errno(arg.ret);
        *ifap = NULL;
        return -1;
    }

    return 0;
}

static void OsFreeifaddrsIteration(struct ifaddrs *ifa)
{
    if (ifa == NULL) {
        return;
    }

    if (ifa->ifa_next != NULL) {
        OsFreeifaddrsIteration(ifa->ifa_next);
    }

    mem_free(ifa);
}

static void OsLwipFreeifaddrs(struct ifaddrs *ifa)
{
    OsFreeifaddrsIteration(ifa);
}

int getifaddrs(struct ifaddrs **ifap)
{
    return OsLwipGetifaddrs(ifap);
}

void freeifaddrs(struct ifaddrs *ifa)
{
    OsLwipFreeifaddrs(ifa);
}

#endif /* (LWIP_IPV4 || LWIP_IPV6) && LWIP_SOCKET */
#endif /* LWIP_IFADDRS */
