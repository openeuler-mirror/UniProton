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
 * Description: 双向链表操作的内部头文件
 */
#ifndef PRT_LIST_EXTERNAL_H
#define PRT_LIST_EXTERNAL_H

#include "prt_typedef.h"
#include "bits/list_types.h"

#define OS_LIST_INIT(head)     \
    do {                       \
        (head)->prev = (head); \
        (head)->next = (head); \
    } while (0)

#define LIST_OBJECT_INIT(object) { \
        &(object), &(object)       \
    }

#define INIT_LIST_OBJECT(object)   \
    do {                           \
        (object)->next = (object); \
        (object)->prev = (object); \
    } while (0)

#define LIST_LAST(object) ((object)->prev)
#define OS_LIST_FIRST(object) ((object)->next)

/* list action low level add */
OS_SEC_ALW_INLINE INLINE void ListLowLevelAdd(struct TagListObject *newNode, struct TagListObject *prev,
                                              struct TagListObject *next)
{
    newNode->next = next;
    newNode->prev = prev;
    next->prev = newNode;
    prev->next = newNode;
}

/* list action add */
OS_SEC_ALW_INLINE INLINE void ListAdd(struct TagListObject *newNode, struct TagListObject *listObject)
{
    ListLowLevelAdd(newNode, listObject, listObject->next);
}

/* list action tail add */
OS_SEC_ALW_INLINE INLINE void ListTailAdd(struct TagListObject *newNode, struct TagListObject *listObject)
{
    ListLowLevelAdd(newNode, listObject->prev, listObject);
}

/* list action lowel delete */
OS_SEC_ALW_INLINE INLINE void ListLowLevelDelete(struct TagListObject *prevNode, struct TagListObject *nextNode)
{
    nextNode->prev = prevNode;
    prevNode->next = nextNode;
}

/* list action delete */
#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE void ListDelAndInit(struct TagListObject *node)
{
    ListLowLevelDelete(node->prev, node->next);
    OS_LIST_INIT(node);
}

OS_SEC_ALW_INLINE INLINE void ListDelete(struct TagListObject *node)
{
    ListDelAndInit(node);
}
#else
OS_SEC_ALW_INLINE INLINE void ListDelete(struct TagListObject *node)
{
    ListLowLevelDelete(node->prev, node->next);

    node->next = NULL;
    node->prev = NULL;
}
#endif

/* list action empty */
#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE bool ListEmpty(const struct TagListObject *listObject)
{
    return (bool)(listObject->next == listObject);
}
#else
OS_SEC_ALW_INLINE INLINE bool ListEmpty(const struct TagListObject *listObject)
{
    return (bool)((listObject->next == listObject) && (listObject->prev == listObject));
}
#endif
#define OFFSET_OF_FIELD(type, field) ((uintptr_t)((uintptr_t)(&((type *)0x10)->field) - (uintptr_t)0x10))

#define COMPLEX_OF(ptr, type, field) ((type *)((uintptr_t)(ptr) - OFFSET_OF_FIELD(type, field)))

/* 根据成员地址得到控制块首地址, ptr成员地址, type控制块结构, field成员名 */
#define LIST_COMPONENT(ptrOfList, typeOfList, fieldOfList) COMPLEX_OF(ptrOfList, typeOfList, fieldOfList)

#define LIST_FOR_EACH(posOfList, listObject, typeOfList, field)                                                    \
    for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field); &(posOfList)->field != (listObject); \
         (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

#define LIST_FIRST_ENTITY(listForFirstEntity, typeOfList, field)    \
    LIST_COMPONENT((listForFirstEntity)->next, typeOfList, field)

#define LIST_FOR_EACH_SAFE(posOfList, listObject, typeOfList, field)                \
    for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field);       \
         (&(posOfList)->field != (listObject))&&((posOfList)->field.next != NULL);  \
         (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

#endif /* PRT_LIST_EXTERNAL_H */
