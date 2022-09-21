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
#ifndef LWIP_PORTING_LWIPOPTS_H
#define LWIP_PORTING_LWIPOPTS_H

#include "prt_config.h"

// lwIP debug options, comment the ones you don't want
#define LWIP_DEBUG 1
#if LWIP_DEBUG
#define ETHARP_DEBUG                    LWIP_DBG_OFF
#define NETIF_DEBUG                     LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define API_LIB_DEBUG                   LWIP_DBG_OFF
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#define ICMP_DEBUG                      LWIP_DBG_OFF
#define IGMP_DEBUG                      LWIP_DBG_OFF
#define INET_DEBUG                      LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_OFF
#define DRIVERIF_DEBUG                  LWIP_DBG_OFF
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                       LWIP_DBG_OFF
#define MEM_DEBUG                       LWIP_DBG_OFF
#define MEMP_DEBUG                      LWIP_DBG_OFF
#define SYS_DEBUG                       LWIP_DBG_ON
#define TIMERS_DEBUG                    LWIP_DBG_OFF
#define TCP_DEBUG                       LWIP_DBG_OFF
#define TCP_ERR_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#define TCP_SACK_DEBUG                  LWIP_DBG_OFF
#define TCP_TLP_DEBUG                   LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define SLIP_DEBUG                      LWIP_DBG_OFF
#define DHCP_DEBUG                      LWIP_DBG_OFF
#define AUTOIP_DEBUG                    LWIP_DBG_OFF
#define DNS_DEBUG                       LWIP_DBG_OFF
#define TFTP_DEBUG                      LWIP_DBG_OFF
#define SYS_ARCH_DEBUG                  LWIP_DBG_OFF
#define SNTP_DEBUG                      LWIP_DBG_OFF
#define IP6_DEBUG                       LWIP_DBG_OFF
#define DHCP6_DEBUG                     LWIP_DBG_OFF
#define DRV_STS_DEBUG                   LWIP_DBG_OFF
#endif

// Options only in new opt.h
#define LWIP_SOCKET_SELECT              1
#define LWIP_SOCKET_POLL                1

// Options in old opt.h that differs from new opt.h
#define MEM_ALIGNMENT                   __SIZEOF_POINTER__
#define MEMP_NUM_NETDB                  8
#define IP_REASS_MAXAGE                 3
#define IP_SOF_BROADCAST                1
#define IP_SOF_BROADCAST_RECV           1
#define LWIP_MULTICAST_PING             1
#define LWIP_RAW                        1
#define LWIP_DHCP_AUTOIP_COOP_TRIES     64
#define TCP_LISTEN_BACKLOG              1
#define TCP_DEFAULT_LISTEN_BACKLOG      16

#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   7

#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_TX_SINGLE_PBUF       1
#define LWIP_NETCONN_FULLDUPLEX         1
#define LWIP_COMPAT_SOCKETS             2
#define LWIP_POSIX_SOCKETS_IO_NAMES     0
#define LWIP_TCP_KEEPALIVE              1
#define RECV_BUFSIZE_DEFAULT            65535
#define SO_REUSE_RXTOALL                1

#define LWIP_CHECKSUM_ON_COPY           1
#ifndef LWIP_IPV6
#define LWIP_IPV6                       1
#endif
#define LWIP_IPV6_NUM_ADDRESSES         5
#define LWIP_ND6_NUM_PREFIXES           10
#define LWIP_IPV6_DHCP6                 1
#define LWIP_IPV6_DHCP6_STATEFUL        1

// Options in old lwipopts.h
#define ARP_QUEUEING                    1
#define DEFAULT_ACCEPTMBOX_SIZE         32
#define DEFAULT_RAW_RECVMBOX_SIZE       128
#define DEFAULT_TCP_RECVMBOX_SIZE       128
#define DEFAULT_UDP_RECVMBOX_SIZE       128
#define ETHARP_SUPPORT_STATIC_ENTRIES   1
#define ETH_PAD_SIZE                    0
#define IP_REASS_MAX_PBUFS              (((65535) / (IP_FRAG_MAX_MTU - 20 - 8) + 1) * MEMP_NUM_REASSDATA)
#define LWIP_COMPAT_SOCKETS             2
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
#ifndef LWIP_DHCP
#define LWIP_DHCP                       0
#endif
#define LWIP_DNS                        1
#define LWIP_ETHERNET                   1
#define LWIP_HAVE_LOOPIF                1
#define LWIP_IGMP                       1
#define LWIP_NETIF_API                  1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_LOOPBACK             1
#define LWIP_POSIX_SOCKETS_IO_NAMES     0
#define LWIP_RAW                        1
#define LWIP_SO_RCVBUF                  1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_SNDTIMEO                1
#define LWIP_STATS_DISPLAY              1
#define MEM_LIBC_MALLOC                 1
#define MEMP_NUM_ARP_QUEUE              (65535 * LWIP_CONFIG_NUM_SOCKETS / (IP_FRAG_MAX_MTU - 20 - 8))
#define MEMP_NUM_NETBUF                 (65535 * 3 * LWIP_CONFIG_NUM_SOCKETS / (IP_FRAG_MAX_MTU - 20 - 8))
#define MEMP_NUM_NETCONN                LWIP_CONFIG_NUM_SOCKETS
#define MEMP_NUM_PBUF                   (LWIP_CONFIG_NUM_SOCKETS*2)
#define MEMP_NUM_RAW_PCB                LWIP_CONFIG_NUM_SOCKETS
#define MEMP_NUM_REASSDATA              (IP_REASS_MAX_MEM_SIZE / 65535)
#define MEMP_NUM_TCPIP_MSG_API          64
#define MEMP_NUM_TCPIP_MSG_INPKT        512
#define MEMP_NUM_TCP_PCB                LWIP_CONFIG_NUM_SOCKETS
#define MEMP_NUM_TCP_PCB_LISTEN         LWIP_CONFIG_NUM_SOCKETS
#define MEMP_NUM_TCP_SEG                (((TCP_SND_BUF * 3 / 2) + TCP_WND) * LWIP_CONFIG_NUM_SOCKETS / TCP_MSS)
#define MEMP_NUM_UDP_PCB                LWIP_CONFIG_NUM_SOCKETS
#define MEM_SIZE                        (4*1024*1024)
#define PBUF_POOL_BUFSIZE               1550
#define PBUF_POOL_SIZE                  64
#define SO_REUSE                        1
#define TCPIP_MBOX_SIZE                 512
#define TCPIP_THREAD_PRIO               5
#define TCPIP_THREAD_STACKSIZE          0x800
#define TCP_MAXRTX                      64
#define TCP_MSS                         1400
#define TCP_SND_BUF                     65535
#define TCP_SND_QUEUELEN                ((8 * TCP_SND_BUF) / TCP_MSS)
#define TCP_TTL                         255
#define TCP_WND                         32768
#define UDP_TTL                         255

// Options in old lwipopts.h but kept in Defaults with new opt.h
#define IP_FORWARD                      0
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON
#define LWIP_ICMP                       1
#define LWIP_NETCONN                    1
#define LWIP_SOCKET                     1
#define LWIP_STATS                      1
#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define NO_SYS                          0
#define TCP_QUEUE_OOSEQ                 LWIP_TCP

#if defined LWIP_STATS_DISPLAY && !LWIP_STATS
#undef LWIP_STATS_DISPLAY
#define LWIP_STATS_DISPLAY              0
#endif

// Change some options for lwIP 2.1.2
#undef TCP_MAXRTX
#define TCP_MAXRTX                      12

#undef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS             0

#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + (LWIP_IPV6 * LWIP_IPV6_DHCP6))

#undef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         LWIP_CONFIG_NUM_SOCKETS

#undef TCP_MSS
#define TCP_MSS                         (IP_FRAG_MAX_MTU - 20 - 20)

#undef IP_SOF_BROADCAST_RECV
#define IP_SOF_BROADCAST_RECV           0

/**
 * Defines whether to enable debugging for driver module.
 */
#ifndef DRIVERIF_DEBUG
#define DRIVERIF_DEBUG                  LWIP_DBG_OFF
#endif

// Options for old lwipopts.h
#define IP_FRAG_MAX_MTU                 1500
#define LWIP_CONFIG_NUM_SOCKETS         128
#define IP_REASS_MAX_MEM_SIZE           (MEM_SIZE / 4)

// Options for enhancement code, same for old lwipopts.h
#define LWIP_NETIF_PROMISC              0
#define LWIP_TFTP                       1
#define LWIP_DHCPS                      1
#define LWIP_ENABLE_NET_CAPABILITY      1
#define LWIP_ENABLE_CAP_NET_BROADCAST   0

#define MEMP_MEM_MALLOC                 1

// Options for TcpTestSelect
#define LWIP_SOCKET_SELECT_FUNC         1

// Options for TcpTestPoll
#define LWIP_SOCKET_POLL_FUNC           1

// use PBUF_RAM instead of PBUF_POOL in udp_input
#define USE_PBUF_RAM_UDP_INPUT          1

#endif /* LWIP_PORTING_LWIPOPTS_H */
