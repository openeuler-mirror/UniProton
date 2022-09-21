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
#include <lwip/etharp.h>
#include <lwip/netifapi.h>
#include <lwip/priv/api_msg.h>

#if LWIP_DHCP
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>

err_t OsDhcpIsBound(struct netif *netif)
{
    struct dhcp *dhcp = NULL;

    LWIP_ERROR("netif != NULL", (netif != NULL), return ERR_ARG);

    dhcp = netif_dhcp_data(netif);
    LWIP_ERROR("netif->dhcp != NULL", (dhcp != NULL), return ERR_ARG);

    if (dhcp->state == DHCP_STATE_BOUND) {
        return ERR_OK;
    } else {
        return ERR_INPROGRESS;
    }
}
#endif /* LWIP_DHCP */

static struct netif *OsNetifFindByName(const char *name)
{
    struct netif *netif = NULL;
    LWIP_ASSERT_CORE_LOCKED();
    if (name == NULL) {
        return NULL;
    }
    NETIF_FOREACH(netif) {
        if (strcmp("lo", name) == 0 && (netif->name[0] == 'l' && netif->name[1] == 'o')) {
            LWIP_DEBUGF(NETIF_DEBUG, ("OsNetifFindByName: found lo\n"));
            return netif;
        }

        if (strcmp(netif->full_name, name) == 0) {
            LWIP_DEBUGF(NETIF_DEBUG, ("OsNetifFindByName: found %s\n", name));
            return netif;
        }
    }
    return NULL;
}

static err_t OsNetifapiDoFindByName(struct tcpip_api_call_data *m)
{
    /* cast through void* to silence alignment warnings.
     * We know it works because the structs have been instantiated as struct netifapi_msg */
    struct netifapi_msg *msg = (struct netifapi_msg *)(void *)m;
    msg->netif = OsNetifFindByName(msg->msg.ifs.name);
    return ERR_OK;
}

struct netif *OsNetifapiNetifFindByName(const char *name)
{
    struct netif *netif = NULL;
    API_VAR_DECLARE(struct netifapi_msg, msg);
    API_VAR_ALLOC(struct netifapi_msg, MEMP_NETIFAPI_MSG, msg, ERR_MEM);
    API_VAR_REF(msg).netif = NULL;
#if LWIP_MPU_COMPATIBLE
    if (strncpy_s(API_VAR_REF(msg).msg.ifs.name, NETIF_NAMESIZE, name, NETIF_NAMESIZE - 1)) {
        API_VAR_FREE(MEMP_NETIFAPI_MSG, msg);
        return netif;
    }
    API_VAR_REF(msg).msg.ifs.name[NETIF_NAMESIZE - 1] = '\0';
#else
    API_VAR_REF(msg).msg.ifs.name = (char *)name;
#endif /* LWIP_MPU_COMPATIBLE */

    (void)tcpip_api_call(OsNetifapiDoFindByName, &API_VAR_REF(msg).call);
    netif = msg.netif;
    API_VAR_FREE(MEMP_NETIFAPI_MSG, msg);

    return netif;
}

#if LWIP_IPV6
int ip6addr_aton(const char *cp, ip6_addr_t *addr)
{
    const S32 ipv6Blocks = 8;
    U16 currentBlockIndex = 0;
    U16 currentBlockValue = 0;
    U16 addr16[ipv6Blocks];
    U16 *a16 = (u16_t *)addr->addr;
    S32 squashPos = ipv6Blocks;
    S32 i;
    S32 num5 = 5;
    S32 num16 = 16;
    const char *sc = (const char *)cp;
    const char *ss = cp - 1;

    for (; ; sc++) {
        if (currentBlockIndex >= ipv6Blocks) {
            return 0; // address too long
        }
        if (*sc == 0) {
            if (sc - ss == 1) {
                if (squashPos != currentBlockIndex) {
                    return 0; // empty address or address ends with a single ':'
                } // else address ends with one valid "::"
            } else {
                addr16[currentBlockIndex++] = currentBlockValue;
            }
            break;
        } else if (*sc == ':') {
            if (sc - ss == 1) {
                if (sc != cp || sc[1] != ':') {
                    return 0; // address begins with a single ':' or contains ":::"
                } // else address begins with one valid "::"
            } else {
                addr16[currentBlockIndex++] = currentBlockValue;
            }
            if (sc[1] == ':') {
                if (squashPos != ipv6Blocks) {
                    return 0; // more than one "::"
                }
                squashPos = currentBlockIndex;
                sc++;
            }
            ss = sc; // ss points to the recent ':' position
            currentBlockValue = 0;
        } else if (lwip_isxdigit(*sc) && (sc - ss) < num5) { // 4 hex-digits at most
            currentBlockValue = (currentBlockValue << 4) +
                (*sc | ('a' - 'A')) - '0' - ('a' - '9' - 1) * (*sc >= 'A');
#if LWIP_IPV4
        } else if (*sc == '.' && currentBlockIndex < ipv6Blocks - 1) {
            ip4_addr_t ip4;
            int ret = ip4addr_aton(ss+1, &ip4);
            if (!ret) {
                return 0;
            }
            ip4.addr = lwip_ntohl(ip4.addr);
            addr16[currentBlockIndex++] = (u16_t)(ip4.addr >> num16);
            addr16[currentBlockIndex++] = (u16_t)(ip4.addr);
            break;
#endif /* LWIP_IPV4 */
        } else {
            return 0; // unexpected char or too many digits
        }
    }

    if (squashPos == ipv6Blocks && currentBlockIndex != ipv6Blocks) {
        return 0; // address too short
    }
    if (squashPos != ipv6Blocks && currentBlockIndex == ipv6Blocks) {
        return 0; // unexpected "::" in address
    }

    for (i = 0; i < squashPos; ++i) {
        a16[i] = lwip_htons(addr16[i]);
    }
    for (; i < ipv6Blocks - currentBlockIndex + squashPos; ++i) {
        a16[i] = 0;
    }
    for (; i < ipv6Blocks; ++i) {
        a16[i] = lwip_htons(addr16[i - ipv6Blocks + currentBlockIndex]);
    }

    return 1;
}
#endif /* LWIP_IPV6 */
