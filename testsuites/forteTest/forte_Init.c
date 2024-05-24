/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-05-06
 * Description: forte demo
 */

#include "forte_Init.h"

void forte_init()
{
    forteGlobalInitialize();
    TForteInstance forteInstance = 0;
    int resultForte = forteStartInstanceGeneric(0, 0, &forteInstance);
    if(FORTE_OK == resultForte){
        forteJoinInstance(forteInstance);
    }else{
        printf("Error %d: Couldn't start forte\n", resultForte);
    }
    forteGlobalDeinitialize();
}