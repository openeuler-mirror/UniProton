/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-10-17
 * Description: PCIE功能
 */

#include "prt_buildef.h"
#include "prt_typedef.h"
#include "pcie_config.h"

bool __attribute__((weak)) pci_bus_accessible(uint32_t bus_no)
{
    if (bus_no >= PCI_BUS_NUM_MAX) {
        return false;
    }
    return true;
}

#if defined(OS_OPTION_PROXY)
struct _IO_FILE { char __x; };
typedef struct _IO_FILE FILE;
FILE *PRT_ProxyPopen(const char *cmd, const char *mode);
size_t PRT_ProxyFreadLoop(void *buffer, size_t size, size_t count, FILE *f);
int PRT_ProxyPclose(FILE *f);
#endif

int __attribute__((weak)) proxybash_exec_lock(char *cmdline, char *result_buf,
    unsigned int buf_len)
{
#if defined(OS_OPTION_PROXY)
    FILE *fp;
    char buffer[0x200];
    int bytes_read = 0;
    int total_bytes_read = 0;
    fp = PRT_ProxyPopen(cmdline, "r");
    if (fp == NULL) {
        return -1;
    }

    while (1) {
        bytes_read = PRT_ProxyFreadLoop(buffer, 1, sizeof(buffer), fp);
        if (bytes_read == 0) {
            break;
        }
        if (total_bytes_read + bytes_read > buf_len) {
            PRT_ProxyPclose(fp);
            return -2;
        }
        memcpy(result_buf + total_bytes_read, buffer, bytes_read);
        total_bytes_read += bytes_read;
    }
    return total_bytes_read;
#else
    /* 依赖功能，该函数如果没有代理文件系统，则需要demo独立实现 */

    return OS_ERROR;
#endif
}