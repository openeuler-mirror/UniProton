/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include "log_list.h"

struct log_list_elem;

struct log_list_elem {
    struct log_list_elem *next;
    struct log_elem_content cont;
};

static struct log_list_elem g_log_list_head;
static struct log_list_elem *g_log_list_tail;

void init_log_list(void)
{
    g_log_list_head.cont.log_mem = NULL;
    g_log_list_head.cont.log_num = 0;
    g_log_list_head.next = NULL;
    g_log_list_tail = &(g_log_list_head);
    LOG_DEBUG("[logList] INIT, head==tail:%p\n", g_log_list_tail);
}

long add_log_elem(struct log_elem_content cont)
{
    struct log_list_elem *elem;
    elem = (struct log_list_elem *)vmalloc(sizeof(struct log_list_elem));
    if (elem == NULL) {
        printk(KERN_ERR "[logWorker] add log element no meme\n");
        return -ENOMEM;
    }
    elem->cont = cont;
    elem->next = NULL;
    LOG_DEBUG("[logList] ADD prev tail:%p new elem:%p\n", g_log_list_tail, elem);
    g_log_list_tail->next = elem;
    g_log_list_tail = elem;
    
    return 0;
}

long peek_log_elem(struct log_elem_content *cont)
{
    struct log_list_elem *elem = g_log_list_head.next;
    if (elem == NULL) {
        return -1;
    }

    if (cont != NULL) {
        *cont = elem->cont;
    }
    return 0;
}

long pop_log_elem(struct log_elem_content *cont)
{
    struct log_list_elem *elem = g_log_list_head.next;
    LOG_DEBUG("[logList] POP head next:%p, tail:%p\n", elem, g_log_list_tail);
    if (elem == NULL) {
        return -1;
    }
    if (cont != NULL) {
        *cont = elem->cont;
    }
    g_log_list_head.next = elem->next;
    vfree(elem);

    // 链表空，尾指针重新指向头指针
    if (g_log_list_head.next == NULL) {
        LOG_DEBUG("[logList] POP Q empty!\n");
        g_log_list_tail = &(g_log_list_head);
    }
    return 0;
}

