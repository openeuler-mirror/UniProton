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

#ifndef LWIP_PORTING_INET_H
#define LWIP_PORTING_INET_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include_next <lwip/inet.h>

#if LWIP_IPV4
#define inet_addr_from_ip4addr(target_inaddr, source_ipaddr) \
            ((target_inaddr)->s_addr = ip4_addr_get_u32(source_ipaddr))
#define inet_addr_to_ip4addr(target_ipaddr, source_inaddr) \
            (ip4_addr_set_u32(target_ipaddr, (source_inaddr)->s_addr))

/* directly map this to the lwip internal functions */
#define inet_ntoa_r(addr, buf, buflen)  ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
#define inet6_addr_from_ip6addr(target_in6addr, source_ip6addr) do {    \
            (target_in6addr)->s6_addr32[0] = (source_ip6addr)->addr[0]; \
            (target_in6addr)->s6_addr32[1] = (source_ip6addr)->addr[1]; \
            (target_in6addr)->s6_addr32[2] = (source_ip6addr)->addr[2]; \
            (target_in6addr)->s6_addr32[3] = (source_ip6addr)->addr[3]; \
} while (0)

#define inet6_addr_to_ip6addr(target_ip6addr, source_in6addr) do {  \
            (target_ip6addr)->addr[0] = (source_in6addr)->s6_addr32[0]; \
            (target_ip6addr)->addr[1] = (source_in6addr)->s6_addr32[1]; \
            (target_ip6addr)->addr[2] = (source_in6addr)->s6_addr32[2]; \
            (target_ip6addr)->addr[3] = (source_in6addr)->s6_addr32[3]; \
            ip6_addr_clear_zone(target_ip6addr);    \
} while (0)

#endif /* LWIP_IPV6 */

#endif /* LWIP_PORTING_INET_H */
