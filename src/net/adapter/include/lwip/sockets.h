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

#ifndef LWIP_PORTING_SOCKETS_H
#define LWIP_PORTING_SOCKETS_H

#include "lwip/arch.h"
#include <sys/socket.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <limits.h>
#include <fcntl.h>
#include_next <lwip/sockets.h>

#if FD_SETSIZE < (LWIP_SOCKET_OFFSET + MEMP_NUM_NETCONN)
#error "external FD_SETSIZE too small for number of sockets"
#else
#define LWIP_SELECT_MAXNFDS FD_SETSIZE
#endif

#if IOV_MAX > 0xFFFF
#error "IOV_MAX larger than supported by LwIP"
#endif

#if LWIP_UDP && LWIP_UDPLITE
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif

// For BSD 4.4 socket sa_len compatibility
#define DF_NADDR(addr)  ip_addr_t naddr = (addr)
#define SA_LEN(addr, _)  (IP_IS_V4_VAL(addr) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))
#define sa_len sa_data[0] * 0 + SA_LEN(naddr, _)
#define sin_len sin_zero[0]
#define sin6_len sin6_addr.s6_addr[0]

// for sockets.c, TCP_KEEPALIVE is not supported currently
#define TCP_KEEPALIVE   0xFF
#define SIN_ZERO_LEN    8

int closesocket(int sockfd);
int ioctlsocket(int s, long cmd, void *argp);

#endif /* LWIP_PORTING_SOCKETS_H */
