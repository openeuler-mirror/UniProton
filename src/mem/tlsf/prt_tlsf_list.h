/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @defgroup los_list Doubly linked list
 * @ingroup kernel
 */

#ifndef _PRT_TLSF_LIST_H
#define _PRT_TLSF_LIST_H

#include "prt_tlsf_compat.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_list
 * Structure of a node in a doubly linked list.
 */
typedef struct OS_TLSF_DL_LIST {
    struct OS_TLSF_DL_LIST *pstPrev; /**< Current node's pointer to the previous node */
    struct OS_TLSF_DL_LIST *pstNext; /**< Current node's pointer to the next node */
} OS_TLSF_DL_LIST;

/**
 * @ingroup los_list
 * @brief Initialize a doubly linked list.
 *
 * @par Description:
 * This API is used to initialize a doubly linked list.
 * @attention
 * <ul>
 * <li>The parameter passed in should be ensured to be a legal pointer.</li>
 * </ul>
 *
 * @param list    [IN] Node in a doubly linked list.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListInit(OS_TLSF_DL_LIST *list)
{
    list->pstNext = list;
    list->pstPrev = list;
}

/**
 * @ingroup los_list
 * @brief Point to the next node pointed to by the current node.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to point to the next node pointed to by the current node.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param object  [IN] Node in the doubly linked list.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_FIRST(object) ((object)->pstNext)

/**
 * @ingroup los_list
 * @brief Insert a new node to a doubly linked list.
 *
 * @par Description:
 * This API is used to insert a new node to a doubly linked list.
 * @attention
 * <ul>
 * <li>The parameters passed in should be ensured to be legal pointers.</li>
 * </ul>
 *
 * @param list    [IN]   Doubly linked list where the new node is inserted.
 * @param node    [IN]   New node to be inserted.
 *
 * @retval None
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see OsTlsfListDelete
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListAdd(OS_TLSF_DL_LIST *list, OS_TLSF_DL_LIST *node)
{
    node->pstNext = list->pstNext;
    node->pstPrev = list;
    list->pstNext->pstPrev = node;
    list->pstNext = node;
}

/**
 * @ingroup los_list
 * @brief Insert a node to the tail of a doubly linked list.
 *
 * @par Description:
 * This API is used to insert a new node to the tail of a doubly linked list.
 * @attention
 * <ul>
 * <li>The parameters passed in should be ensured to be legal pointers.</li>
 * </ul>
 *
 * @param list     [IN] Doubly linked list where the new node is inserted.
 * @param node     [IN] New node to be inserted.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see OsTlsfListAdd | OsTlsfListHeadInsert
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListTailInsert(OS_TLSF_DL_LIST *list, OS_TLSF_DL_LIST *node)
{
    OsTlsfListAdd(list->pstPrev, node);
}

/**
 * @ingroup los_list
 * @brief Insert a node to the head of a doubly linked list.
 *
 * @par Description:
 * This API is used to insert a new node to the head of a doubly linked list.
 * @attention
 * <ul>
 * <li>The parameters passed in should be ensured to be legal pointers.</li>
 * </ul>
 *
 * @param list     [IN] Doubly linked list where the new node is inserted.
 * @param node     [IN] New node to be inserted.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see OsTlsfListAdd | OsTlsfListTailInsert
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListHeadInsert(OS_TLSF_DL_LIST *list, OS_TLSF_DL_LIST *node)
{
    OsTlsfListAdd(list, node);
}

/**
 * @ingroup los_list
 * @brief Delete a specified node from a doubly linked list.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to delete a specified node from a doubly linked list.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The parameter passed in should be ensured to be a legal pointer.</li>
 * </ul>
 *
 * @param node    [IN] Node to be deleted.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see OsTlsfListAdd
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListDelete(OS_TLSF_DL_LIST *node)
{
    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;
    node->pstNext = (OS_TLSF_DL_LIST *)NULL;
    node->pstPrev = (OS_TLSF_DL_LIST *)NULL;
}

/**
 * @ingroup los_list
 * @brief Identify whether a specified doubly linked list is empty.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to return whether a doubly linked list is empty.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The parameter passed in should be ensured to be a legal pointer.</li>
 * </ul>
 *
 * @param list  [IN] Doubly linked node.
 *
 * @retval TRUE The doubly linked list is empty.
 * @retval FALSE The doubly linked list is not empty.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE BOOL OsTlsfListEmpty(OS_TLSF_DL_LIST *node)
{
    return (BOOL)(node->pstNext == node);
}

/**
 * @ingroup los_list
 * @brief Obtain the pointer to a doubly linked list in a structure.
 *
 * @par Description:
 * This API is used to obtain the pointer to a doubly linked list in a structure.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param type    [IN] Structure name.
 * @param member  [IN] Member name of the doubly linked list in the structure.
 *
 * @retval Pointer to the doubly linked list in the structure.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_OFF_SET_OF(type, member) ((UINT32)&(((type *)0)->member))

/**
 * @ingroup los_list
 * @brief Obtain the pointer to a structure that contains a doubly linked list.
 *
 * @par Description:
 * This API is used to obtain the pointer to a structure that contains a doubly linked list.
 * <ul>
 * <li>None.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param item    [IN] Current node's pointer to the next node.
 * @param type    [IN] Structure name.
 * @param member  [IN] Member name of the doubly linked list in the structure.
 *
 * @retval Pointer to the structure that contains the doubly linked list.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_ENTRY(item, type, member) \
    ((type *)(VOID *)((CHAR *)(item) - OS_TLSF_OFF_SET_OF(type, member))) \

/**
 * @ingroup los_list
 * @brief Iterate over a doubly linked list of given type.
 *
 * @par Description:
 * This API is used to iterate over a doubly linked list of given type.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param item           [IN] Pointer to the structure that contains the doubly linked list that is to be traversed.
 * @param list           [IN] Pointer to the doubly linked list to be traversed.
 * @param type           [IN] Structure name.
 * @param member         [IN] Member name of the doubly linked list in the structure.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_FOR_EACH_ENTRY(item, list, type, member) \
    for ((item) = OS_TLSF_DL_LIST_ENTRY((list)->pstNext, type, member); \
            &(item)->member != (list); \
            (item) = OS_TLSF_DL_LIST_ENTRY((item)->member.pstNext, type, member))

/**
 * @ingroup los_list
 * @brief iterate over a doubly linked list safe against removal of list entry.
 *
 * @par Description:
 * This API is used to iterate over a doubly linked list safe against removal of list entry.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param item           [IN] Pointer to the structure that contains the doubly linked list that is to be traversed.
 * @param next           [IN] Save the next node.
 * @param list           [IN] Pointer to the doubly linked list to be traversed.
 * @param type           [IN] Structure name.
 * @param member         [IN] Member name of the doubly linked list in the structure.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, list, type, member) \
    for ((item) = OS_TLSF_DL_LIST_ENTRY((list)->pstNext, type, member), \
            (next) = OS_TLSF_DL_LIST_ENTRY((item)->member.pstNext, type, member); \
            &((item)->member) != (list); \
            (item) = (next), (next) = OS_TLSF_DL_LIST_ENTRY((item)->member.pstNext, type, member))

/**
 * @ingroup los_list
 * @brief Delete initialize a doubly linked list.
 *
 * @par Description:
 * This API is used to delete initialize a doubly linked list.
 * @attention
 * <ul>
 * <li>The parameter passed in should be ensured to be s legal pointer.</li>
 * </ul>
 *
 * @param list    [IN] Doubly linked list.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
PRT_TLSF_SEC_ALW_INLINE STATIC_INLINE VOID OsTlsfListDelInit(OS_TLSF_DL_LIST *list)
{
    list->pstNext->pstPrev = list->pstPrev;
    list->pstPrev->pstNext = list->pstNext;
    OsTlsfListInit(list);
}

/**
 * @ingroup los_list
 * @brief iterate over a doubly linked list.
 *
 * @par Description:
 * This API is used to iterate over a doubly linked list.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param item           [IN] Pointer to the structure that contains the doubly linked list that is to be traversed.
 * @param list           [IN] Pointer to the doubly linked list to be traversed.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_FOR_EACH(item, list) \
    for ((item) = (list)->pstNext; (item) != (list); (item) = (item)->pstNext)

/**
 * @ingroup los_list
 * @brief Iterate over a doubly linked list safe against removal of list entry.
 *
 * @par Description:
 * This API is used to iterate over a doubly linked list safe against removal of list entry.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param item           [IN] Pointer to the structure that contains the doubly linked list that is to be traversed.
 * @param next           [IN] Save the next node.
 * @param list           [IN] Pointer to the doubly linked list to be traversed.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_FOR_EACH_SAFE(item, next, list) \
    for ((item) = (list)->pstNext, (next) = (item)->pstNext; (item) != (list); \
            (item) = (next), (next) = (item)->pstNext)

/**
 * @ingroup los_list
 * @brief Initialize a double linked list.
 *
 * @par Description:
 * This API is used to initialize a double linked list.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param list           [IN] Pointer to the doubly linked list to be traversed.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>prt_tlsf_list.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define OS_TLSF_DL_LIST_HEAD(list) \
    OS_TLSF_DL_LIST list = { &(list), &(list) }

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _PRT_TLSF_LIST_H */
