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
 * Create: 2024-07-4
 * Description: 网络
 */
#ifndef __RESGISTER_H__
#define __RESGISTER_H__
#include "lwip/netif.h"

struct ethernet_api {
    int (*init)(struct netif* netif);
    int (*send)(struct netif* netif, const unsigned char *packet, int length);
    int (*recv)(struct netif* netif, unsigned char *packet, int length);
};

int ethernetif_api_register(struct ethernet_api *api);
#endif