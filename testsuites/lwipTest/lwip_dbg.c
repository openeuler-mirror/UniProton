/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-06-12
 * Description: i210网卡注册lwip
 */

#include <string.h>
#include "securec.h"
#include <pthread.h>
#include "prt_typedef.h"
#include "lwip/sockets.h"
#include "lwip/etharp.h"
#include "net.h"
#include "i210.h"

static pthread_t td;
char i210_mac[6] = {0xb0,0x41,0x6f,0x03,0xc6,0x4b};

#define IFNAME0 'e'
#define IFNAME1 't'

int EthernetInit(struct netif* netif)
{
    i210_init();
    memcpy(netif->hwaddr, i210_mac, ETH_HWADDR_LEN);
    netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
}

int EthernetSend(struct netif* netif, const unsigned char *packet, int length)
{
    i210_packet_send(packet, length);
}

int EthernetRecv(struct netif* netif, unsigned char *packet, int length)
{
    i210_packet_recv(packet, length);
}

struct ethernet_api ethInterface(void)
{
    static struct ethernet_api ethApi = {
        .init = EthernetInit,
        .send = EthernetSend,
        .recv = EthernetRecv,
    };

    return ethApi;
}

static void *lwip_udp_test(void *arg)
{    
    // lwip udp client
    lwip_test_udp();
}

void lwip_test_start(void)
{
    printf("lwip Test Start\n");
    // 代理网络
    proxy_udp_client();

    struct ethernet_api reg_api = ethInterface();
    ethernetif_api_register(&reg_api);

    lwipInit();
    int ret = pthread_create(&td, NULL, lwip_udp_test, NULL);
}

void lwip_test_stop(void)
{
    printf("lwip Test Stop\n");
    pthread_cancel(td);
}