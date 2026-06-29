#ifndef PRT_BESTFIT_LIST_H
#define PRT_BESTFIT_LIST_H

#include "prt_bestfit_compat.h"

typedef struct PRT_DL_LIST {
    struct PRT_DL_LIST *pstPrev;
    struct PRT_DL_LIST *pstNext;
} PRT_DL_LIST;

PRT_BESTFIT_SEC_ALW_INLINE STATIC INLINE VOID PRT_ListInit(PRT_DL_LIST *list)
{
    list->pstNext = list;
    list->pstPrev = list;
}

PRT_BESTFIT_SEC_ALW_INLINE STATIC INLINE VOID PRT_ListAdd(PRT_DL_LIST *list, PRT_DL_LIST *node)
{
    node->pstNext = list->pstNext;
    node->pstPrev = list;
    list->pstNext->pstPrev = node;
    list->pstNext = node;
}

PRT_BESTFIT_SEC_ALW_INLINE STATIC INLINE VOID PRT_ListTailInsert(PRT_DL_LIST *list, PRT_DL_LIST *node)
{
    PRT_ListAdd(list->pstPrev, node);
}

PRT_BESTFIT_SEC_ALW_INLINE STATIC INLINE VOID PRT_ListDelete(PRT_DL_LIST *node)
{
    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;
    node->pstNext = NULL;
    node->pstPrev = NULL;
}

#define PRT_OFF_SET_OF(type, member) ((UINTPTR)&(((type *)0)->member))
#define PRT_DL_LIST_ENTRY(item, type, member) \
    ((type *)(VOID *)((CHAR *)(item) - PRT_OFF_SET_OF(type, member)))
#define PRT_DL_LIST_FOR_EACH_ENTRY(item, list, type, member) \
    for ((item) = PRT_DL_LIST_ENTRY((list)->pstNext, type, member); \
         &((item)->member) != (list); \
         (item) = PRT_DL_LIST_ENTRY((item)->member.pstNext, type, member))

#endif /* PRT_BESTFIT_LIST_H */
