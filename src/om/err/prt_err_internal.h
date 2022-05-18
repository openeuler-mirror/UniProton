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
 * Description: 错误模块的模块内头文件
 */
#ifndef PRT_ERR_INTERNAL_H
#define PRT_ERR_INTERNAL_H

#include "prt_hook_external.h"

/*
 * 模块内宏定义
 */
#define OS_ERR_CALL_USR_HOOK(fileName, lineNo, errorNo, paraLen, para) \
    OS_SHOOK_ACTIVATE_PARA5(OS_HOOK_ERR_REG, (fileName), (lineNo), (errorNo), (paraLen), (para))

#endif /* PRT_ERR_INTERNAL_H */
