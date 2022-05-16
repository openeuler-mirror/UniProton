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
 * Description: 错误处理函数文件
 */
#include "prt_err.h"
#include "prt_attr_external.h"
#include "prt_hook_external.h"

/*
 * 描述：错误处理用户钩子函数注册
 */
OS_SEC_L4_TEXT U32 PRT_ErrRegHook(ErrHandleFunc hook)
{
    return OsShookReg(OS_HOOK_ERR_REG, (OsVoidFunc)hook);
}
