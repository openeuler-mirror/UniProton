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
 *
 * Author: niutao2@huawei.com
 * Create: 2023-09-20
 * Description: 树莓派gdb资源配置
 */

#include "prt_typedef.h"
#include "cpu_config.h"
#include "prt_gdbstub_ext.h"

extern const char __os_section_start[];
extern const char __os_section_end[];
extern const char __os_stub_data_start[];
extern const char __os_stub_data_end[];
extern const char __os_stub_text_start[];
extern const char __os_stub_text_end[];

static STUB_DATA struct GdbMemRegion g_regions[] = {
    {
        .start = (uintptr_t)__os_section_start,
        .end = (uintptr_t)__os_section_end,
        .attributes = GDB_MEM_REGION_RW
    },
    {
        .start = (uintptr_t)__os_stub_text_start,
        .end = (uintptr_t)__os_stub_text_end,
        .attributes = GDB_MEM_REGION_NO_SWBKPT
    },
    {
        .start = (uintptr_t)__os_stub_data_start,
        .end = (uintptr_t)__os_stub_data_end,
        .attributes = GDB_MEM_REGION_NO_SWBKPT
    }
};

static STUB_DATA struct GdbRingBufferCfg g_rbufCfg = {
    .txaddr = MMU_GDB_STUB_ADDR,
    .rxaddr = MMU_GDB_STUB_ADDR + 0x1000,
    .size = 0x1000
};


STUB_TEXT int OsGdbConfigGetMemRegions(struct GdbMemRegion **regions)
{
    if (!regions) {
        return 0;
    }
    *regions = g_regions;
    return sizeof(g_regions) / sizeof(struct GdbMemRegion);
}

STUB_TEXT struct GdbRingBufferCfg *OsGetGdbRingBufferCfg()
{
    return &g_rbufCfg;
}