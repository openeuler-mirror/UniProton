/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-12
 * Description: 通知链实现
 */
#include "prt_typedef.h"
#include "prt_notifier.h"
#include "prt_hwi.h"

static struct NotifierHead g_dieNotifier = {
    .head = NULL
};

static int NotifierChainRegister(struct NotifierBlock **nl,
        struct NotifierBlock *n)
{
    while ((*nl) != NULL) {
        if (*nl == n) {
            // double register
            return -1;
        }
        if (n->priority > (*nl)->priority) {
            break;
        }
        nl = &((*nl)->next);
    }
    n->next = *nl;
    *nl = n;
    return 0;
}

static int NotifierChainUnregister(struct NotifierBlock **nl,
        struct NotifierBlock *n)
{
    while (*nl != NULL) {
        if (*nl == n) {
            *nl = n->next;
            return 0;
        }
        nl = &((*nl)->next);
    }
    return -1;
}

static int NotifierCallChain(struct NotifierBlock **nl,
                   int val, void *v,
                   int nr_to_call, int *nr_calls)
{
    int ret = NOTIFY_DONE;
    struct NotifierBlock *nb, *next_nb;

    nb = *nl;

    while (nb && nr_to_call) {
        next_nb = nb->next;

        ret = nb->call(nb, val, v);

        if (nr_calls) {
            (*nr_calls)++;
        }

        if (ret & NOTIFY_STOP_MASK) {
            break;
        }
        nb = next_nb;
        nr_to_call--;
    }
    return ret;
}

int OsNotifierChainRegister(struct NotifierHead *nh,
        struct NotifierBlock *n)
{
    uintptr_t flags;
    int ret;

    flags = PRT_HwiLock();
    ret = NotifierChainRegister(&nh->head, n);
    PRT_HwiRestore(flags);
    return ret;
}

int OsNotifierChainUnregister(struct NotifierHead *nh,
        struct NotifierBlock *n)
{
    uintptr_t flags;
    int ret;

    flags = PRT_HwiLock();
    ret = NotifierChainUnregister(&nh->head, n);
    PRT_HwiRestore(flags);
    return ret;
}

int OsNotifierCallChain(struct NotifierHead *nh,
                   int val, void *v,
                   int nr_to_call, int *nr_calls)
{
    uintptr_t flags;
    int ret;

    flags = PRT_HwiLock();
    ret = NotifierCallChain(&nh->head, val, v, -1, NULL);
    PRT_HwiRestore(flags);
    return ret;
}

int OsNotifierRawCallChain(struct NotifierHead *nh,
                   int val, void *v,
                   int nr_to_call, int *nr_calls)
{
    return NotifierCallChain(&nh->head, val, v, -1, NULL);
}

int OsNotifyDie(int val, void *v)
{
    return NotifierCallChain(&g_dieNotifier.head, val, v, -1, NULL);
}

int OsRegisterDieNotifier(struct NotifierBlock *nb)
{
    return OsNotifierChainRegister(&g_dieNotifier, nb);
}

int OsUnregisterDieNotifier(struct NotifierBlock *nb)
{
    return OsNotifierChainUnregister(&g_dieNotifier, nb);
}
