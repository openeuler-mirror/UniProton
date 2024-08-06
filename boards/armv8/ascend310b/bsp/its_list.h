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
 * Create: 2023-11-30
 * Description: 链表相关操作
 */

#ifndef __ITS_LIST_H__
#define __ITS_LIST_H__

#include <stddef.h>
#include <stdbool.h>

struct list_node
{
    struct list_node *prev;
    struct list_node *next;
};

#define container_of(ptr, type, member) \
  ((type *)((uintptr_t)(ptr) - offsetof(type, member)))
  
#define list_initialize(list) \
  do \
    { \
      struct list_node *__list = (list); \
      __list->prev = __list; \
      __list->next = __list; \
    } \
  while(0)

#define list_add_tail(list, item) \
  do \
    { \
        struct list_node *__list = (list); \
        struct list_node *__item = (item); \
      __item->prev       = __list->prev; \
      __item->next       = __list; \
      __list->prev->next = __item; \
      __list->prev       = __item; \
    } \
  while (0)

#define list_for_every_entry(list, entry, type, member) \
  for(entry = container_of((list)->next, type, member); \
      &entry->member != (list); \
      entry = container_of(entry->member.next, type, member))

#endif