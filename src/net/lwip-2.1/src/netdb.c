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
#include "lwip/netdb.h"

#if LWIP_DNS && LWIP_SOCKET

struct hostent *gethostbyname(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    return lwip_gethostbyname(name);
}

int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen,
                                struct hostent **result, int *h_errnop)
{
    return lwip_gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

void freeaddrinfo(struct addrinfo *res)
{
    lwip_freeaddrinfo(res);
}

int getaddrinfo(const char *restrict nodename, const char *restrict servname,
                            const struct addrinfo *restrict hints, struct addrinfo **restrict res)
{
    return lwip_getaddrinfo(nodename, servname, hints, res);
}

#endif
