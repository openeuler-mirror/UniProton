/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: System base function implementation
 */
#include "securec.h"
#include "prt_sys.h"
#include "prt_cpu_external.h"

#define OS_VER "UniProton 24.03-LTS"

/* 宏展开优先展开参数，但是#会阻止参数展开，故需要定义另外一个宏 */
OS_SEC_DATA char g_osOuterVer[OS_SYS_OS_VER_LEN] = { OS_VER };
/*
 * 描述：获取OS版本号
 */
OS_SEC_L4_TEXT char *PRT_SysGetOsVersion(void)
{
    return (char *)&g_osOuterVer[0];
}
