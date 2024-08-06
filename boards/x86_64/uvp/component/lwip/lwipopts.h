/**
 * @file
 *
 * lwIP Options Configuration
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <errno.h>

#define ETHARP_TRUST_IP_MAC    0
#define IP_REASSEMBLY          1
#define IP_FRAG                1
#define ARP_QUEUEING           0
#define TCP_LISTEN_BACKLOG     1

#define LWIP_IPV6 0
#define LWIP_IPV4 1

/**
* SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
* critical regions during buffer allocation, deallocation and memory
* allocation and deallocation.
*/
#define SYS_LIGHTWEIGHT_PROT    1

/**
* NO_SYS==1: Provides VERY minimal functionality. Otherwise,
* use lwIP facilities.
*/
#define NO_SYS                  0

/**
* NO_SYS_NO_TIMERS==1: Drop support for sys_timeout when NO_SYS==1
* Mainly for compatibility to old versions.
*/
#define NO_SYS_NO_TIMERS        0

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
    lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
    byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (10*1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
    sends a lot of data out of ROM (or other static memory), this
    should be set high. */
#define MEMP_NUM_PBUF           100
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
    per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        8
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
    connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
    connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
    segments. */
#define MEMP_NUM_TCP_SEG        12
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
    timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    10

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          20

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       500

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
    order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40) /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (4*TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)

/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
turning this on does currently not work. */
#define LWIP_DHCP               0

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* ---------- Statistics options ---------- */
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1

/* ---------- link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
* whenever the link changes (i.e., link down)
*/
#define LWIP_NETIF_LINK_CALLBACK        1
/*
    --------------------------------------
    ---------- Checksum options ----------
    --------------------------------------
*/

/*The STM32F4x7 allows comput
ing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
- To use this feature let the following define uncommented.
- To disable it and process by CPU comment the  the checksum.
*/
#if defined(USE_PPPOS) && (USE_PPPOS == 1)
// for the ppp support
#define PPP_SUPPORT         1
#define PAP_SUPPORT         1
#define PPP_IPV4_SUPPORT    1
#endif

#undef CHECKSUM_BY_HARDWARE


#ifdef CHECKSUM_BY_HARDWARE
 /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
 #define CHECKSUM_GEN_IP                 0
 /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
 #define CHECKSUM_GEN_UDP                0
 /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
 #define CHECKSUM_GEN_TCP                0
 /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
 #define CHECKSUM_CHECK_IP               0
 /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
 #define CHECKSUM_CHECK_UDP              0
 /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
 #define CHECKSUM_CHECK_TCP              0
 /*CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
 #define CHECKSUM_GEN_ICMP               0
#else
 /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
 #define CHECKSUM_GEN_IP                 1
 /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
 #define CHECKSUM_GEN_UDP                1
 /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
 #define CHECKSUM_GEN_TCP                1
 /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
 #define CHECKSUM_CHECK_IP               1
 /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
 #define CHECKSUM_CHECK_UDP              1
 /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
 #define CHECKSUM_CHECK_TCP              1
 /*CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
 #define CHECKSUM_GEN_ICMP               1
#endif

/*
    ----------------------------------------------
    ---------- Sequential layer options ----------
    ----------------------------------------------
*/
/**
* LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
*/
#define LWIP_NETCONN                    1

/*
    ------------------------------------
    ---------- Socket options ----------
    ------------------------------------
*/
/**
* LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
*/
#define LWIP_SOCKET                     1

/*
------------------------------------
---------- dns options ----------
------------------------------------
*/
/*
* LWIP_DNS==1: Enable Domain Name System
*/
#define LWIP_DNS                       1

/*
------------------------------------
---------- httpd options ----------
------------------------------------
*/
/*
* set this to 1 to include "fsdata_custom.c" instead of "fsdata.c"
*/
#define HTTPD_USE_CUSTOM_FSDATA        0

/*
----------------------------------------------
---------- Lwip Dtls Server options ----------
----------------------------------------------
*/
/*
* set this to 1 to include "fsdata_custom.c" instead of "fsdata.c"
*/
#define SO_REUSE                       1

#ifndef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS  1
#define LWIP_POSIX_SOCKETS_IO_NAMES  0
#define LWIP_FCNTL  1
#endif

#ifndef LWIP_SOCKET_POLL
#define LWIP_SOCKET_POLL     1
#endif

#ifndef LWIP_SO_RCVTIMEO
#define LWIP_SO_RCVTIMEO     1
#endif

#if LWIP_DNS
#ifndef LWIP_DNS_REVERSE
#define LWIP_DNS_REVERSE     1
#endif
#else
#undef LWIP_DNS_REVERSE
#define LWIP_DNS_REVERSE     0
#endif

#define LWIP_STATIC static

/* *Defines the maximum number of IP addresses supported.* */
#ifndef DNS_MAX_IPADDR
#define DNS_MAX_IPADDR                 10
#endif

/* *Defines the maximum DNS label length as per RFC.* */
#ifndef DNS_MAX_LABEL_LENGTH
#define DNS_MAX_LABEL_LENGTH           63
#endif

#ifndef CONFIG_NFILE_DESCRIPTORS
#define CONFIG_NFILE_DESCRIPTORS       256
#endif

#ifndef LWIP_RAW
#define LWIP_RAW  1
#endif

#ifndef LWIP_NETIF_IFINDEX_MAX
#define LWIP_NETIF_IFINDEX_MAX 0xFE
#endif

#ifndef LWIP_NETIF_API
#define LWIP_NETIF_API      1
#endif

#define LWIP_IOCTL_IF       1

#ifndef DEFAULT_RAW_RECVMBOX_SIZE
#define DEFAULT_RAW_RECVMBOX_SIZE      16
#endif

#ifndef LWIP_MAX_PF_RAW_SEND_SIZE
#define LWIP_MAX_PF_RAW_SEND_SIZE      (0xFFFF - 40) /* IPv6 header */
#endif

#ifndef LWIP_MAX_UDP_RAW_SEND_SIZE
#define LWIP_MAX_UDP_RAW_SEND_SIZE      65432
#endif

#ifndef LWIP_NETIF_IFINDEX_START
#define LWIP_NETIF_IFINDEX_START        1
#endif

#ifndef NUM_SOCKETS
#define NUM_SOCKETS MEMP_NUM_NETCONN
#endif

#if LWIP_RAW
#undef PF_PACKET_SOCKET
#define PF_PACKET_SOCKET           0
#else
#undef PF_PACKET_SOCKET
#define PF_PACKET_SOCKET           0
#endif

#ifndef LWIP_NETIF_LOOPBACK
#define LWIP_NETIF_LOOPBACK        1
#endif

#define LWIP_INET_ADDR_FUNC        1

/*
------------------------------------
---------- IGMP options ----------
------------------------------------
*/
/*
* LWIP_IGMP == 1: Turn on IGMP module.
*/
#ifndef LWIP_IGMP
#define LWIP_IGMP                  1
#endif

/*
------------------------------------
----------OS options ----------
------------------------------------
*/

#define TCPIP_THREAD_NAME                "TCP/IP"
#define TCPIP_THREAD_STACKSIZE           0x1000
#define TCPIP_MBOX_SIZE                  12
#define DEFAULT_UDP_RECVMBOX_SIZE        12
#define DEFAULT_TCP_RECVMBOX_SIZE        6
#define DEFAULT_ACCEPTMBOX_SIZE          6
#define DEFAULT_THREAD_STACKSIZE         0x1000
#define TCPIP_THREAD_PRIO                6

#define LWIP_COMPAT_MUTEX_ALLOWED        1
#define LWIP_COMPAT_MUTEX                0

#define LWIP_DEBUG
#define ETHARP_DEBUG     LWIP_DBG_ON
#define PBUF_DEBUG       LWIP_DBG_ON
#define API_LIB_DEBUG    LWIP_DBG_ON
#define API_MSG_DEBUG    LWIP_DBG_ON
#define NETIF_DEBUG      LWIP_DBG_ON
#define INET_DEBUG       LWIP_DBG_ON
#define DHCP_DEBUG       LWIP_DBG_ON
#define IGMP_DEBUG       LWIP_DBG_ON
#define MDNS_DEBUG       LWIP_DBG_ON
#define UDP_DEBUG        LWIP_DBG_ON
#define SNTP_DEBUG       LWIP_DBG_ON
#define SYS_DEBUG        LWIP_DBG_ON
#define SOCKETS_DEBUG    LWIP_DBG_ON
#define TCP_DEBUG        LWIP_DBG_ON
#define HTTPC_DEBUG      LWIP_DBG_ON
#endif /* __LWIPOPTS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/