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
 * Create: 2023-08-31
 * Description: dl_iterate_phdr
 */

#include <elf.h>
#include <link.h>
#include "pthread_impl.h"
#include "libc.h"

#define MAX_PHDRS 8
static __attribute__((__section__(".data"))) ElfW(Phdr) g_phdrs[MAX_PHDRS];

int static_dl_iterate_phdr(int(*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data)
{
    struct dl_phdr_info info;

    info.dlpi_addr  = 0;
    info.dlpi_name  = "";
    info.dlpi_phdr  = g_phdrs;
    info.dlpi_phnum = MAX_PHDRS;
    info.dlpi_adds  = 0;
    info.dlpi_subs  = 0;
    info.dlpi_tls_modid = 0;
    info.dlpi_tls_data = 0;

    return (callback)(&info, sizeof(info), data);
}

weak_alias(static_dl_iterate_phdr, dl_iterate_phdr);
