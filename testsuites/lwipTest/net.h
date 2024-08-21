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
 * Create: 2024-06-13
 * Description: lwip注册初始化头文件
 */

#ifndef NET_TEST_H
#define NET_TEST_H

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "arch/net_register.h"

void lwipInit();
void lwip_test_udp();
int proxy_udp_client();

#endif /* _UNIT_TEST_H_ */