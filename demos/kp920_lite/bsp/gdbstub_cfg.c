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
 * Author: wangyouwang@huawei.com
 * Create: 2023-09-20
 * Description: KP920服务器gdb资源配置
 */

#include "prt_typedef.h"
#include "cpu_config.h"
#include "prt_gdbstub_ext.h"

static STUB_DATA struct GdbRingBufferCfg g_rbufCfg = {
    .txaddr = MMU_GDB_STUB_ADDR,
    .rxaddr = MMU_GDB_STUB_ADDR + 0x1000,
    .size = 0x1000
};

STUB_TEXT struct GdbRingBufferCfg *OsGetGdbRingBufferCfg()
{
    return &g_rbufCfg;
}