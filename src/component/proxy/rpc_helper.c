/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-30
 * Description: 辅助函数，主要用于序列化和反序列化
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "securec.h"
#include "rpc_internal_model.h"
#include "rpc_err.h"

static char *__strdup(const char *s)
{
    size_t l = strlen(s);
    char *d = malloc(l + 1);
    if (d == NULL) {
        return NULL;
    }
    memcpy_s(d, l + 1, s, l + 1);
    return d;
}

void OsProxyFreeAddrList(struct addrinfo *ai)
{
    struct addrinfo *p = NULL;

    while (ai != NULL) {
        p = ai;
        ai = ai->ai_next;
        free(p->ai_canonname);
        free(p);
    }
}

int OsProxyEncodeAddrList(const struct addrinfo *ai, char *buf, int *buflen)
{
    int len = 0, bi = 0, cnt = 0, aclen = 0;
    const struct addrinfo *p = ai;
    int hlen = sizeof(iaddrinfo_t) - sizeof(int);

    if (ai == NULL || buf == NULL || buflen == NULL) {
        return -RPC_EINVAL;
    }
    while (p != NULL) {
        len += hlen + p->ai_addrlen + sizeof(int);
        if (p->ai_canonname != NULL) {
            len += strlen(p->ai_canonname) + 1;
        }
        p = p->ai_next;
    }
    if (len > *buflen) {
        return -RPC_EOVERLONG;
    }
    *buflen = len;
    p = ai;
    while (p != NULL) {
        memcpy_s(&buf[bi], hlen, p, hlen);
        bi += hlen;
        aclen = 0;
        if (p->ai_canonname != NULL) {
            aclen = strlen(p->ai_canonname) + 1;
        }
        memcpy_s(&buf[bi], sizeof(int), &aclen, sizeof(int));
        bi += sizeof(int);
        if (p->ai_addr != NULL && p->ai_addrlen > 0) {
            memcpy_s(&buf[bi], p->ai_addrlen, p->ai_addr, p->ai_addrlen);
            bi += p->ai_addrlen;
        }
        if (aclen > 0) {
            memcpy_s(&buf[bi], aclen, p->ai_canonname, aclen);
            bi += aclen;
        }
        p = p->ai_next;
        cnt++;
    }

    return cnt;
}

int OsProxyDecodeAddrList(const char *buf, int cnt, int buflen, struct addrinfo **out)
{
    int bi = 0, aclen = 0, ret = 0;
    struct addrinfo *p = NULL, **pp = out;
    int hlen = sizeof(iaddrinfo_t) - sizeof(int);

    *out = p;
    for (int i = 0; i < cnt; i++) {
        struct addrinfo addr;
        memcpy_s(&addr, hlen, &buf[bi], hlen);
        *pp = p = (struct addrinfo *)malloc(sizeof(struct addrinfo) + addr.ai_addrlen);
        if (p == NULL) {
            ret = -RPC_ENOMEM;
            goto clean;
        }
        if (bi + hlen >= buflen) {
            ret = -RPC_EOVERLONG;
            goto clean;
        }
        memcpy_s(p, hlen, &buf[bi], hlen);
        bi += hlen;
        if (bi + sizeof(int) >= buflen) {
            ret = -RPC_EOVERLONG;
            goto clean;
        }
        memcpy_s(&aclen, sizeof(int), &buf[bi], sizeof(int));
        bi += sizeof(int);
        p->ai_addr = (void *)&p[1];
        if (addr.ai_addrlen > 0) {
            if (bi + addr.ai_addrlen >= buflen) {
                ret = -RPC_EOVERLONG;
                goto clean;
            }
            memcpy_s(p->ai_addr, addr.ai_addrlen, &buf[bi], addr.ai_addrlen);
            bi += addr.ai_addrlen;
        }
        p->ai_canonname = NULL;
        if (aclen > 0) {
            if (&buf[bi] == NULL) {
                ret = -RPC_ECORRUPTED;
                goto clean;
            }
            p->ai_canonname = __strdup(&buf[bi]);
            bi += aclen;
        }

        p->ai_next = NULL;
        pp = &(p->ai_next);
    }
    return 0;
clean:
    OsProxyFreeAddrList(*out);
    return ret;
}

int OsProxyDecodeHostEnt(struct hostent **ppht, char *src_buf, int buflen)
{
    ihostent_t ih;
    int tlen = 0, dst_idx = 0, src_idx = 0, i = 0, slen = 0;
    struct hostent *pht = NULL;
    char *dst_buf, **aliases, **addr;
    if (ppht == NULL || src_buf == NULL || buflen < sizeof(ihostent_t)) {
        return -RPC_EINVAL;
    }
    memcpy_s(&ih, sizeof(ih), src_buf, sizeof(ih));
    if (ih.h_name_idx > ih.h_aliases_idx ||
        ih.h_aliases_idx > ih.h_addr_list_idx) {
        return -RPC_ECORRUPTED;
    }
    tlen = buflen + sizeof(char *) * (ih.aliaslen + ih.addrlen + 2) -
    sizeof(ihostent_t) + sizeof(struct hostent);
    dst_idx = sizeof(char *) * (ih.aliaslen + ih.addrlen + 2);
    src_idx = sizeof(ihostent_t);
    pht = (struct hostent *)malloc(tlen);
    if (pht == NULL) {
        return -RPC_ENOMEM;
    }
    dst_buf = (char *)(pht + 1);
    memcpy_s(&dst_buf[dst_idx], buflen - sizeof(ihostent_t), 
             &src_buf[src_idx], buflen - sizeof(ihostent_t));

    pht->h_length = ih.h_length;
    pht->h_addrtype = ih.h_addrtype;
    if (ih.h_name_idx == ih.h_aliases_idx) {
        pht->h_name = NULL;
    } else {
        pht->h_name = &dst_buf[dst_idx];
        dst_idx += ih.h_aliases_idx - ih.h_name_idx;
    }
    aliases = pht->h_aliases = (char **)dst_buf;
    for (i = 0; i < ih.aliaslen; i++) {
        aliases[i] = &dst_buf[dst_idx];
        if (aliases[i] == NULL) {
            free(pht);
            return -RPC_ECORRUPTED;
        }
        slen = strlen(aliases[i]) + 1;
        dst_idx += slen;
    }
    aliases[i] = NULL;
    addr = pht->h_addr_list = (char **)&dst_buf[sizeof(char *) * (ih.aliaslen + 1)];
    slen = pht->h_length;
    for (i = 0 ; i < ih.addrlen; i++) {
        addr[i] = &dst_buf[dst_idx];
        dst_idx += slen;
    }
    addr[i] = NULL;
    *ppht = pht;
    return 0;
}

int OsProxyEncodeHostEnt(struct hostent *ht, char *buf, int buflen)
{
    int tlen = sizeof(ihostent_t), len = 0;
    ihostent_t ih;
    char **p;

    if (ht == NULL || buf == NULL) {
        return -RPC_EINVAL;
    }
    ih.aliaslen = 0;
    ih.addrlen = 0;
    ih.h_name_idx = tlen;
    ih.h_addrtype = ht->h_addrtype;
    ih.h_length = ht->h_length;
    if (ht->h_name != NULL) {
        len = strlen(ht->h_name) + 1;
        if (tlen + len >= buflen) {
            return -RPC_EOVERLONG;
        }
        memcpy_s(&buf[tlen], len, ht->h_name, len);
        tlen += len;
    }
    ih.h_aliases_idx = tlen;
    if (ht->h_aliases != NULL) {
        p = ht->h_aliases;
        for (int i = 0; p[i] != NULL; i++) {
            len = strlen(p[i]) + 1;
            if (tlen + len >= buflen) {
                return -RPC_EOVERLONG;
            }
            memcpy_s(&buf[tlen], len, p[i], len);
            ih.aliaslen++;
            tlen += len;
        }
    }
    ih.h_addr_list_idx = tlen;
    if (ht->h_addr_list != NULL) {
        len = ht->h_length;
        p = ht->h_addr_list;
        for (int i = 0; p[i] != NULL; i++) {
            if (tlen + len >= buflen) {
                return -RPC_EOVERLONG;
            }
            memcpy_s(&buf[tlen], len, p[i], len);
            ih.addrlen++;
            tlen += len;
        }
    }
    memcpy_s(buf, len, &ih, sizeof(ih));
    return tlen;
}
