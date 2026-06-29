/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2019. All rights reserved.
 * Description: LiteOS Mem Module Implementation
 * Author: Huawei LiteOS Team
 * Create: 2013-01-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 * --------------------------------------------------------------------------- */

#include "prt_bestfit_memory_internal.h"
#include "prt_bestfit_memory_pri.h"

#ifdef PRT_BESTFIT_CFG_LIB_LIBC
#include "stdio.h"
#endif
#include "prt_bestfit_multipledlinkhead_pri.h"
#include "prt_bestfit_slab_pri.h"
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
#include "prt_bestfit_memrecord_pri.h"
#endif
#include "prt_bestfit_task_pri.h"
#include "prt_bestfit_exc.h"
#include "prt_bestfit_spinlock.h"
#include "prt_bestfit_trace.h"

#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
#include "prt_bestfit_exc_pri.h"
#endif

#include "prt_bestfit_lms_pri.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define NODEDUMPSIZE  64  /* the dump size of current broken node when memcheck error */
#define COLUMN_NUM    8   /* column num of the output info of mem node */

#define OS_CHECK_NULL_RETURN(param) do {              \
    if ((param) == NULL) {                            \
        PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__); \
        return;                                       \
    }                                                 \
} while (0)

/* spinlock for mem module, only available on SMP mode */
PRT_BESTFIT_SEC_BSS  SPIN_LOCK_INIT(g_memSpin);

UINT8 *m_aucSysMem0 = NULL;
UINT8 *m_aucSysMem1 = NULL;

MALLOC_HOOK g_MALLOC_HOOK = NULL;

__attribute__((section(".data.init"))) UINTPTR g_sys_mem_addr_end;
__attribute__((section(".data.init"))) UINTPTR g_excInteractMemSize = 0;

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_SIZE_CHECK
STATIC UINT8 g_memCheckLevel = PRT_BESTFIT_MEM_CHECK_LEVEL_DEFAULT;
#endif

#ifdef PRT_BESTFIT_CFG_MEM_MUL_MODULE
UINT32 g_moduleMemUsedSize[MEM_MODULE_MAX + 1] = { 0 };
#endif

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
STATIC VOID OsMemNodeSave(PrtBestfitMemDynNode *node);
#endif

STATIC VOID *OsMemAllocWithCheck(PrtBestfitMemPoolInfo *pool, UINT32 size);

STATIC INLINE VOID OS_MEM_TASKID_SET(PrtBestfitMemDynNode *node, UINT32 taskId)
{
    node->selfNode.taskId = taskId;

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
}

STATIC INLINE UINT32 OS_MEM_TASKID_GET(const PrtBestfitMemDynNode *node)
{
    return (UINT32)node->selfNode.taskId;
}

#ifdef PRT_BESTFIT_CFG_MEM_MUL_MODULE

STATIC INLINE VOID OS_MEM_MODID_SET(PrtBestfitMemDynNode *node, UINT32 moduleId)
{
    node->selfNode.moduleId = moduleId;

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
}

STATIC INLINE UINT32 OS_MEM_MODID_GET(const PrtBestfitMemDynNode *node)
{
    return (UINT32)node->selfNode.moduleId;
}

#endif

/*
 * Description : set magic & taskid
 * Input       : node -- the node which will be set magic & taskid
 */
STATIC INLINE VOID OsMemSetMagicNumAndTaskID(PrtBestfitMemDynNode *node)
{
#if defined(PRT_BESTFIT_CFG_MEM_DEBUG) || defined(PRT_BESTFIT_MEM_TASK_STAT)
    PrtBestfitTaskCB *runTask = PrtBestfitCurrTaskGet();

    OS_MEM_SET_MAGIC(node->selfNode.magic);

    /*
     * If the operation occurred before task initialization(runTask was not assigned)
     * or in interrupt, make the value of taskid of node to  (TASK_NUM - 1) which is
     * the id of the extra task. We can record those memory use with that.
     */
    if ((runTask != NULL) && OS_INT_INACTIVE) {
        OS_MEM_TASKID_SET(node, runTask->taskId);
    } else {
        /* If the task mode does not initialize, the field is the 0xffffffff */
        OS_MEM_TASKID_SET(node, TASK_NUM - 1);
    }
#endif
}

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_SIZE_CHECK
const VOID *OsMemFindNodeCtrl(const VOID *pool, const VOID *ptr);
#endif
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
#define CHECKSUM_MAGICNUM    0xDEADBEEF
#define OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode)    \
    (((UINTPTR)(ctlNode)->freeNodeInfo.pstPrev) ^  \
    ((UINTPTR)(ctlNode)->freeNodeInfo.pstNext) ^   \
    ((UINTPTR)(ctlNode)->preNode) ^                \
    (ctlNode)->gapSize ^                           \
    (ctlNode)->sizeAndFlag ^                       \
    CHECKSUM_MAGICNUM)

STATIC INLINE VOID OsMemDispCtlNode(const PrtBestfitMemCtlNode *ctlNode)
{
    UINTPTR checksum;

    checksum = OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);

    PRINTK("node:%p checksum=%p[%p] freeNodeInfo.pstPrev=%p "
           "freeNodeInfo.pstNext=%p preNode=%p gapSize=0x%x sizeAndFlag=0x%x\n",
           ctlNode,
           ctlNode->checksum,
           checksum,
           ctlNode->freeNodeInfo.pstPrev,
           ctlNode->freeNodeInfo.pstNext,
           ctlNode->preNode,
           ctlNode->gapSize,
           ctlNode->sizeAndFlag);
}

STATIC INLINE VOID OsMemDispMoreDetails(const PrtBestfitMemDynNode *node)
{
    UINT32 taskId;
    PrtBestfitTaskCB *taskCB = NULL;

    PRINT_ERR("************************************************\n");
    OsMemDispCtlNode(&node->selfNode);
    PRINT_ERR("the address of node :%p\n", node);

    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        PRINT_ERR("this is a FREE node\n");
        PRINT_ERR("************************************************\n\n");
        return;
    }

    taskId = OS_MEM_TASKID_GET(node);
    if (taskId >= g_taskMaxNum) {
        PRINT_ERR("The task [ID: 0x%x] is ILLEGAL\n", taskId);
        if (taskId == g_taskMaxNum) {
            PRINT_ERR("PROBABLY alloc by SYSTEM INIT, NOT IN ANY TASK\n");
        }
        PRINT_ERR("************************************************\n\n");
        return;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    if ((taskCB->taskStatus & OS_TASK_STATUS_UNUSED) ||
        (taskCB->taskEntry == NULL) ||
        (taskCB->taskName == NULL)) {
        PRINT_ERR("The task [ID: 0x%x] is NOT CREATED(ILLEGAL)\n", taskId);
        PRINT_ERR("************************************************\n\n");
        return;
    }

    PRINT_ERR("allocated by task: %s [ID = 0x%x]\n", taskCB->taskName, taskId);
#ifdef PRT_BESTFIT_CFG_MEM_MUL_MODULE
    PRINT_ERR("allocated by moduleId: %lu\n", OS_MEM_MODID_GET(node));
#endif

    PRINT_ERR("************************************************\n\n");
}

STATIC INLINE VOID OsMemDispWildPointerMsg(const PrtBestfitMemDynNode *node, const VOID *ptr)
{
    PRINT_ERR("*****************************************************\n");
    PRINT_ERR("find a control block at: %p, gap size: 0x%x, sizeof(PrtBestfitMemDynNode): 0x%x\n", node,
              node->selfNode.gapSize, sizeof(PrtBestfitMemDynNode));
    PRINT_ERR("the pointer should be: %p\n",
              ((UINTPTR)node + node->selfNode.gapSize + sizeof(PrtBestfitMemDynNode)));
    PRINT_ERR("the pointer given is: %p\n", ptr);
    PRINT_ERR("PROBABLY A WILD POINTER\n");
    OsBackTrace();
    PRINT_ERR("*****************************************************\n\n");
}

STATIC INLINE VOID OsMemChecksumSet(PrtBestfitMemCtlNode *ctlNode)
{
    ctlNode->checksum = OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);
}

STATIC INLINE BOOL OsMemChecksumVerify(const PrtBestfitMemCtlNode *ctlNode)
{
    return ctlNode->checksum == OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);
}

STATIC INLINE VOID OsMemBackupSetup(const PrtBestfitMemDynNode *node)
{
    PrtBestfitMemDynNode *nodePre = node->selfNode.preNode;
    if (nodePre != NULL) {
        nodePre->backupNode.freeNodeInfo.pstNext = node->selfNode.freeNodeInfo.pstNext;
        nodePre->backupNode.freeNodeInfo.pstPrev = node->selfNode.freeNodeInfo.pstPrev;
        nodePre->backupNode.preNode = node->selfNode.preNode;
        nodePre->backupNode.checksum = node->selfNode.checksum;
        nodePre->backupNode.gapSize = node->selfNode.gapSize;
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        nodePre->backupNode.originSize = node->selfNode.originSize;
#endif
        nodePre->backupNode.sizeAndFlag = node->selfNode.sizeAndFlag;
    }
}

PrtBestfitMemDynNode *OsMemNodeNextGet(const VOID *pool, const PrtBestfitMemDynNode *node)
{
    const PrtBestfitMemPoolInfo *poolInfo = (const PrtBestfitMemPoolInfo *)pool;

    if (node == OS_MEM_END_NODE(pool, poolInfo->poolSize)) {
        return OS_MEM_FIRST_NODE(pool);
    } else {
        return OS_MEM_NEXT_NODE(node);
    }
}

STATIC INLINE UINT32 OsMemBackupSetup4Next(const VOID *pool, PrtBestfitMemDynNode *node)
{
    PrtBestfitMemDynNode *nodeNext = OsMemNodeNextGet(pool, node);

    if (!OsMemChecksumVerify(&nodeNext->selfNode)) {
        PRINT_ERR("[%s]the next node is broken!!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodeNext->selfNode));
        PRINT_ERR("Current node details:\n");
        OsMemDispMoreDetails(node);

        return PRT_NOK;
    }

    if (!OsMemChecksumVerify(&node->backupNode)) {
        node->backupNode.freeNodeInfo.pstNext = nodeNext->selfNode.freeNodeInfo.pstNext;
        node->backupNode.freeNodeInfo.pstPrev = nodeNext->selfNode.freeNodeInfo.pstPrev;
        node->backupNode.preNode = nodeNext->selfNode.preNode;
        node->backupNode.checksum = nodeNext->selfNode.checksum;
        node->backupNode.gapSize = nodeNext->selfNode.gapSize;
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        node->backupNode.originSize = nodeNext->selfNode.originSize;
#endif
        node->backupNode.sizeAndFlag = nodeNext->selfNode.sizeAndFlag;
    }
    return PRT_OK;
}

UINT32 OsMemBackupDoRestore(VOID *pool, const PrtBestfitMemDynNode *nodePre, PrtBestfitMemDynNode *node)
{
    if (node == NULL) {
        PRINT_ERR("the node is NULL.\n");
        return PRT_NOK;
    }
    PRINT_ERR("the backup node information of current node in previous node:\n");
    OsMemDispCtlNode(&nodePre->backupNode);
    PRINT_ERR("the detailed information of previous node:\n");
    OsMemDispMoreDetails(nodePre);

    node->selfNode.freeNodeInfo.pstNext = nodePre->backupNode.freeNodeInfo.pstNext;
    node->selfNode.freeNodeInfo.pstPrev = nodePre->backupNode.freeNodeInfo.pstPrev;
    node->selfNode.preNode = nodePre->backupNode.preNode;
    node->selfNode.checksum = nodePre->backupNode.checksum;
    node->selfNode.gapSize = nodePre->backupNode.gapSize;
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    node->selfNode.originSize = nodePre->backupNode.originSize;
#endif
    node->selfNode.sizeAndFlag = nodePre->backupNode.sizeAndFlag;

    /* we should re-setup next node's backup on current node */
    return OsMemBackupSetup4Next(pool, node);
}

STATIC PrtBestfitMemDynNode *OsMemFirstNodePrevGet(const PrtBestfitMemPoolInfo *poolInfo)
{
    PrtBestfitMemDynNode *nodePre = NULL;

    nodePre = OS_MEM_END_NODE(poolInfo, poolInfo->poolSize);
    if (!OsMemChecksumVerify(&(nodePre->selfNode))) {
        PRINT_ERR("the current node is THE FIRST NODE !\n");
        PRINT_ERR("[%s]: the node information of previous node is bad !!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodePre->selfNode));
    }
    if (!OsMemChecksumVerify(&(nodePre->backupNode))) {
        PRINT_ERR("the current node is THE FIRST NODE !\n");
        PRINT_ERR("[%s]: the backup node information of current node in previous Node is bad !!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodePre->backupNode));
        return NULL;
    }

    return nodePre;
}

PrtBestfitMemDynNode *OsMemNodePrevGet(VOID *pool, const PrtBestfitMemDynNode *node)
{
    PrtBestfitMemDynNode *nodeCur = NULL;
    PrtBestfitMemDynNode *nodePre = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;

    if (node == OS_MEM_FIRST_NODE(pool)) {
        return OsMemFirstNodePrevGet(poolInfo);
    }

    for (nodeCur = OS_MEM_FIRST_NODE(pool);
         nodeCur < OS_MEM_END_NODE(pool, poolInfo->poolSize);
         nodeCur = OS_MEM_NEXT_NODE(nodeCur)) {
        if (!OsMemChecksumVerify(&(nodeCur->selfNode))) {
            PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->selfNode));

            if (nodePre == NULL) {
                return NULL;
            }

            PRINT_ERR("the detailed information of previous node:\n");
            OsMemDispMoreDetails(nodePre);

            /* due to the every step's checksum verify, nodePre is trustful */
            if (OsMemBackupDoRestore(pool, nodePre, nodeCur) != PRT_OK) {
                return NULL;
            }
        }

        if (!OsMemChecksumVerify(&(nodeCur->backupNode))) {
            PRINT_ERR("[%s]: the backup node information in current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->backupNode));

            if (nodePre != NULL) {
                PRINT_ERR("the detailed information of previous node:\n");
                OsMemDispMoreDetails(nodePre);
            }

            if (OsMemBackupSetup4Next(pool, nodeCur) != PRT_OK) {
                return NULL;
            }
        }

        if (OS_MEM_NEXT_NODE(nodeCur) == node) {
            return nodeCur;
        }

        if (OS_MEM_NEXT_NODE(nodeCur) > node) {
            break;
        }

        nodePre = nodeCur;
    }

    return NULL;
}

PrtBestfitMemDynNode *OsMemNodePrevTryGet(VOID *pool, PrtBestfitMemDynNode **node, const VOID *ptr)
{
    UINTPTR nodeShouldBe;
    PrtBestfitMemDynNode *nodeCur = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PrtBestfitMemDynNode *nodePre = OS_MEM_END_NODE(pool, poolInfo->poolSize);

    for (nodeCur = OS_MEM_FIRST_NODE(pool);
         nodeCur < OS_MEM_END_NODE(pool, poolInfo->poolSize);
         nodeCur = OS_MEM_NEXT_NODE(nodeCur)) {
        if (!OsMemChecksumVerify(&(nodeCur->selfNode))) {
            PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->selfNode));

            PRINT_ERR("the detailed information of previous node:\n");
            OsMemDispMoreDetails(nodePre);

            /* due to the every step's checksum verify, nodePre is trustful */
            if (OsMemBackupDoRestore(pool, nodePre, nodeCur) != PRT_OK) {
                return NULL;
            }
        }

        if (!OsMemChecksumVerify(&(nodeCur->backupNode))) {
            PRINT_ERR("[%s]: the backup node information in current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->backupNode));

            if (nodePre != NULL) {
                PRINT_ERR("the detailed information of previous node:\n");
                OsMemDispMoreDetails(nodePre);
            }

            if (OsMemBackupSetup4Next(pool, nodeCur) != PRT_OK) {
                return NULL;
            }
        }

        nodeShouldBe = (UINTPTR)nodeCur + nodeCur->selfNode.gapSize + sizeof(PrtBestfitMemDynNode);
        if (nodeShouldBe == (UINTPTR)ptr) {
            *node = nodeCur;
            return nodePre;
        }

        if (OS_MEM_NEXT_NODE(nodeCur) > (PrtBestfitMemDynNode *)ptr) {
            OsMemDispWildPointerMsg(nodeCur, ptr);
            break;
        }

        nodePre = nodeCur;
    }

    return NULL;
}

STATIC INLINE UINT32 OsMemBackupTryRestore(VOID *pool, PrtBestfitMemDynNode **node, const VOID *ptr)
{
    PrtBestfitMemDynNode *nodeHead = NULL;
    PrtBestfitMemDynNode *nodePre = OsMemNodePrevTryGet(pool, &nodeHead, ptr);
    if (nodePre == NULL) {
        return PRT_NOK;
    }

    *node = nodeHead;
    return OsMemBackupDoRestore(pool, nodePre, *node);
}

STATIC INLINE UINT32 OsMemBackupRestore(VOID *pool, PrtBestfitMemDynNode *node)
{
    PrtBestfitMemDynNode *nodePre = OsMemNodePrevGet(pool, node);
    if (nodePre == NULL) {
        return PRT_NOK;
    }

    return OsMemBackupDoRestore(pool, nodePre, node);
}

STATIC INLINE UINT32 OsMemBackupCheckAndRetore(VOID *pool, VOID *ptr, PrtBestfitMemDynNode *node)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PrtBestfitMemDynNode *startNode = OS_MEM_FIRST_NODE(pool);
    PrtBestfitMemDynNode *endNode   = OS_MEM_END_NODE(pool, poolInfo->poolSize);

    if (OS_MEM_MIDDLE_ADDR(startNode, node, endNode)) {
        /* GapSize is bad or node is broken, we need to verify & try to restore */
        if (!OsMemChecksumVerify(&(node->selfNode))) {
            node = (PrtBestfitMemDynNode *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
            return OsMemBackupTryRestore(pool, &node, ptr);
        }
    }
    return PRT_OK;
}

STATIC INLINE VOID OsMemSetGapSize(PrtBestfitMemCtlNode *ctlNode, UINT32 gapSize)
{
    ctlNode->gapSize = gapSize;
}

STATIC VOID OsMemNodeSave(PrtBestfitMemDynNode *node)
{
    OsMemSetGapSize(&(node->selfNode), 0);
    OsMemChecksumSet(&(node->selfNode));
    OsMemBackupSetup(node);
}

STATIC VOID OsMemNodeSaveWithGapSize(PrtBestfitMemDynNode *node, UINT32 gapSize)
{
    OsMemSetGapSize(&(node->selfNode), gapSize);
    OsMemChecksumSet(&(node->selfNode));
    OsMemBackupSetup(node);
}

STATIC VOID OsMemListDelete(PRT_DL_LIST *node, const VOID *firstNode)
{
    PrtBestfitMemDynNode *dynNode = NULL;

    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;

    if ((VOID *)(node->pstNext) >= firstNode) {
        dynNode = PRT_DL_LIST_ENTRY(node->pstNext, PrtBestfitMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    if ((VOID *)(node->pstPrev) >= firstNode) {
        dynNode = PRT_DL_LIST_ENTRY(node->pstPrev, PrtBestfitMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    node->pstNext = NULL;
    node->pstPrev = NULL;

    dynNode = PRT_DL_LIST_ENTRY(node, PrtBestfitMemDynNode, selfNode.freeNodeInfo);
    OsMemNodeSave(dynNode);
}

STATIC VOID OsMemListAdd(PRT_DL_LIST *listNode, PRT_DL_LIST *node, const VOID *firstNode)
{
    PrtBestfitMemDynNode *dynNode = NULL;

    node->pstNext = listNode->pstNext;
    node->pstPrev = listNode;

    dynNode = PRT_DL_LIST_ENTRY(node, PrtBestfitMemDynNode, selfNode.freeNodeInfo);
    OsMemNodeSave(dynNode);

    listNode->pstNext->pstPrev = node;
    if ((VOID *)(listNode->pstNext) >= firstNode) {
        dynNode = PRT_DL_LIST_ENTRY(listNode->pstNext, PrtBestfitMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    listNode->pstNext = node;
}

VOID PRT_BestfitMemBadNodeShow(VOID *pool)
{
    PrtBestfitMemDynNode *nodePre = NULL;
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 intSave;

    if (pool == NULL) {
        return;
    }

    MEM_LOCK(intSave);

    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= OS_MEM_END_NODE(pool, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        OsMemDispCtlNode(&tmpNode->selfNode);

        if (OsMemChecksumVerify(&tmpNode->selfNode)) {
            continue;
        }

        nodePre = OsMemNodePrevGet(pool, tmpNode);
        if (nodePre == NULL) {
            PRINT_ERR("the current node is invalid, but cannot find its previous Node\n");
            continue;
        }

        PRINT_ERR("the detailed information of previous node:\n");
        OsMemDispMoreDetails(nodePre);
    }

    MEM_UNLOCK(intSave);
    PRINTK("check finish\n");
}

#else  /* without PRT_BESTFIT_CFG_MEM_HEAD_BACKUP */

STATIC VOID OsMemListDelete(PRT_DL_LIST *node, const VOID *firstNode)
{
    (VOID)firstNode;
    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;
    node->pstNext = NULL;
    node->pstPrev = NULL;
}

STATIC VOID OsMemListAdd(PRT_DL_LIST *listNode, PRT_DL_LIST *node, const VOID *firstNode)
{
    (VOID)firstNode;
    node->pstNext = listNode->pstNext;
    node->pstPrev = listNode;
    listNode->pstNext->pstPrev = node;
    listNode->pstNext = node;
}

#endif

#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
__attribute__((always_inline)) inline VOID OsMemLinkRegisterRecord(PrtBestfitMemDynNode *node)
{
    UINT32 count = 0;
    UINT32 index = 0;
    UINTPTR framePtr, tmpFramePtr, linkReg;

    (VOID)memset_s(node->selfNode.linkReg, (PRT_BESTFIT_RECORD_LR_CNT * sizeof(UINTPTR)), 0,
        (PRT_BESTFIT_RECORD_LR_CNT * sizeof(UINTPTR)));
    framePtr = ArchGetFp();
    while ((framePtr > OS_SYS_FUNC_ADDR_START) && (framePtr < OS_SYS_FUNC_ADDR_END)) {
        tmpFramePtr = framePtr;
#ifdef __LP64__
        framePtr = *(UINTPTR *)framePtr;
        linkReg = *(UINTPTR *)(tmpFramePtr + sizeof(UINTPTR));
#else
        linkReg = *(UINTPTR *)framePtr;
        framePtr = *(UINTPTR *)(tmpFramePtr - sizeof(UINTPTR));
#endif
        if (index >= PRT_BESTFIT_OMIT_LR_CNT) {
            node->selfNode.linkReg[count++] = linkReg;
            if (count == PRT_BESTFIT_RECORD_LR_CNT) {
                break;
            }
        }
        index++;
    }
    return;
}

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_INTEGRITY_CHECK
STATIC VOID OsMemNodeBacktraceInfo(const PrtBestfitMemDynNode *tmpNode, const PrtBestfitMemDynNode *preNode)
{
    INT32 i;
    PRINTK("\n broken node head LR info: \n");
    for (i = 0; i < PRT_BESTFIT_RECORD_LR_CNT; i++) {
        PRINTK(" LR[%d]:%p\n", i, tmpNode->selfNode.linkReg[i]);
    }
    PRINTK("\n pre node head LR info: \n");
    for (i = 0; i < PRT_BESTFIT_RECORD_LR_CNT; i++) {
        PRINTK(" LR[%d]:%p\n", i, preNode->selfNode.linkReg[i]);
    }

#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
    WriteExcInfoToBuf("\n broken node head LR info: \n");
    for (i = 0; i < PRT_BESTFIT_RECORD_LR_CNT; i++) {
        WriteExcInfoToBuf("LR[%d]:%p\n", i, tmpNode->selfNode.linkReg[i]);
    }
    WriteExcInfoToBuf("\n pre node head LR info: \n");
    for (i = 0; i < PRT_BESTFIT_RECORD_LR_CNT; i++) {
        WriteExcInfoToBuf("LR[%d]:%p\n", i, preNode->selfNode.linkReg[i]);
    }
#endif
}
#endif

PRT_BESTFIT_SEC_TEXT_MINOR VOID OsMemUsedNodeShow(VOID *pool)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 intSave;
    UINT32 count;

    if (pool == NULL) {
        PRINTK("input param is NULL\n");
        return;
    }
    if (PRT_BestfitMemIntegrityCheck(pool)) {
        PRINTK("PRT_BestfitMemIntegrityCheck error\n");
        return;
    }
    MEM_LOCK(intSave);
#ifdef __LP64__
    PRINTK("node                ");
#else
    PRINTK("node        ");
#endif
    for (count = 0; count < PRT_BESTFIT_RECORD_LR_CNT; count++) {
#ifdef __LP64__
        PRINTK("        LR[%u]       ", count);
#else
        PRINTK("    LR[%u]   ", count);
#endif
    }
    PRINTK("\n");
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode < OS_MEM_END_NODE(pool, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
#ifdef __LP64__
            PRINTK("%018p: ", tmpNode);
#else
            PRINTK("%010p: ", tmpNode);
#endif
            for (count = 0; count < PRT_BESTFIT_RECORD_LR_CNT; count++) {
#ifdef __LP64__
                PRINTK(" %018p ", tmpNode->selfNode.linkReg[count]);
#else
                PRINTK(" %010p ", tmpNode->selfNode.linkReg[count]);
#endif
            }
            PRINTK("\n");
        }
    }
    MEM_UNLOCK(intSave);
    return;
}

#endif

#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
STATIC INLINE VOID OsMemReallocNodeRecord(PrtBestfitMemDynNode *node, UINT32 size, const VOID *ptr)
{
    node->selfNode.originSize = size;
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
    OsMemRecordMalloc(ptr, size);
}
#endif

#ifdef PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_EXTENTION

STATIC VOID *OsMemReallocSlab(VOID *pool, VOID *ptr, BOOL *isSlabMem, UINT32 size)
{
    errno_t rc;
    UINT32 blkSz;
    VOID *newPtr = NULL;
    VOID *freePtr = ptr;

    blkSz = OsSlabMemCheck(pool, ptr);
    if (blkSz == (UINT32)-1) {
        *isSlabMem = FALSE;
        return NULL;
    }
    *isSlabMem = TRUE;

    if (size <= blkSz) {
        return ptr;
    }

    /* Unlock the memory spin, to allow the memory alloc API to be called */
    PRT_SpinUnlock(&g_memSpin);

    if (newPtr == NULL) {
        newPtr = PRT_BestfitMemAlloc(pool, size);
    }

    if (newPtr != NULL) {
        rc = memcpy_s(newPtr, size, ptr, blkSz);
        if (rc != EOK) {
            PRINT_ERR("%s[%d] memcpy_s failed, error type = %d\n", __FUNCTION__, __LINE__, rc);
            freePtr = newPtr;
            newPtr = NULL;
        }
        if (PRT_BestfitMemFree((VOID *)pool, freePtr) != PRT_OK) {
            PRINT_ERR("%s, %d\n", __FUNCTION__, __LINE__);
        }
    }

    /* Reacquire the spin */
    PRT_SpinLock(&g_memSpin);

    return newPtr;
}

VOID *OsBestfitHeapAlloc(VOID *pool, UINT32 size)
{
    return OsMemAllocWithCheck(pool, size);
}

#else

STATIC VOID *OsMemReallocSlab(VOID *pool, const VOID *ptr, BOOL *isSlabMem, UINT32 size)
{
    *isSlabMem = FALSE;
    return NULL;
}

#endif

#ifdef PRT_BESTFIT_CFG_EXC_INTERACTION
PRT_BESTFIT_SEC_TEXT_INIT UINT32 OsMemExcInteractionInit(UINTPTR memStart)
{
    UINT32 ret;
    m_aucSysMem0 = (UINT8 *)((memStart + (POOL_ADDR_ALIGNSIZE - 1)) & ~((UINTPTR)(POOL_ADDR_ALIGNSIZE - 1)));
    g_excInteractMemSize = EXC_INTERACT_MEM_SIZE;
    ret = PRT_BestfitMemInit(m_aucSysMem0, g_excInteractMemSize);
    PRINT_INFO("UniProton bestfit kernel exc interaction memory address:%p,size:0x%x\n",
        m_aucSysMem0, g_excInteractMemSize);
    return ret;
}
#endif

PRT_BESTFIT_SEC_TEXT_INIT UINT32 OsMemSystemInit(UINTPTR memStart)
{
    UINT32 ret;
    UINT32 poolSize;

    m_aucSysMem1 = (UINT8 *)((memStart + (POOL_ADDR_ALIGNSIZE - 1)) & ~((UINTPTR)(POOL_ADDR_ALIGNSIZE - 1)));
    poolSize = OS_SYS_MEM_SIZE;
    ret = PRT_BestfitMemInit(m_aucSysMem1, poolSize);
    PRINT_INFO("UniProton bestfit system heap memory address:%p,size:0x%x\n", m_aucSysMem1, poolSize);
#ifndef PRT_BESTFIT_CFG_EXC_INTERACTION
    m_aucSysMem0 = m_aucSysMem1;
#endif
    return ret;
}

/*
 * Description : find suitable free block use "best fit" algorithm
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- Size of memory in bytes which note need allocate
 * Return      : NULL      --- no suitable block found
 *               tmpNode   --- pointer a suitable free block
 */
STATIC INLINE PrtBestfitMemDynNode *OsMemFindSuitableFreeBlock(VOID *pool, UINT32 allocSize)
{
    PRT_DL_LIST *listNodeHead = NULL;
    PrtBestfitMemDynNode *tmpNode = NULL;

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    UINT32 ret = PRT_OK;
#endif
    for (listNodeHead = OS_MEM_HEAD(pool, allocSize); listNodeHead != NULL;
         listNodeHead = OsDLnkNextMultiHead(OS_MEM_HEAD_ADDR(pool), listNodeHead)) {
        PRT_DL_LIST_FOR_EACH_ENTRY(tmpNode, listNodeHead, PrtBestfitMemDynNode, selfNode.freeNodeInfo) {
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
            if (!OsMemChecksumVerify(&tmpNode->selfNode)) {
                PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
                OsMemDispCtlNode(&tmpNode->selfNode);
                ret = OsMemBackupRestore(pool, tmpNode);
            }
            if (ret != PRT_OK) {
                break;
            }
#endif

#ifdef PRT_BESTFIT_CFG_MEM_DEBUG
            if (((UINTPTR)tmpNode < (UINTPTR)pool) ||
                ((UINTPTR)tmpNode > ((UINTPTR)pool + ((PrtBestfitMemPoolInfo *)pool)->poolSize)) ||
                (((UINTPTR)tmpNode & (OS_MEM_ALIGN_SIZE - 1)) != 0)) {
                PRT_Panic("[%s:%d]Mem node data error:OS_MEM_HEAD_ADDR(pool)=%p, listNodeHead:%p,"
                          "allocSize=%u, tmpNode=%p\n",
                          __FUNCTION__, __LINE__, OS_MEM_HEAD_ADDR(pool), listNodeHead, allocSize, tmpNode);
                break;
            }
#endif
            if (tmpNode->selfNode.sizeAndFlag >= allocSize) {
                return tmpNode;
            }
        }
    }

    return NULL;
}

/*
 * Description : clear a mem node, set every member to NULL
 * Input       : node    --- Pointer to the mem node which will be cleared up
 */
STATIC INLINE VOID OsMemClearNode(PrtBestfitMemDynNode *node)
{
    (VOID)memset_s((VOID *)node, sizeof(PrtBestfitMemDynNode), 0, sizeof(PrtBestfitMemDynNode));
}

/*
 * Description : merge this node and pre node, then clear this node info
 * Input       : node    --- Pointer to node which will be merged
 */
STATIC INLINE VOID OsMemMergeNode(PrtBestfitMemDynNode *node)
{
    PrtBestfitMemDynNode *nextNode = NULL;

    node->selfNode.preNode->selfNode.sizeAndFlag += node->selfNode.sizeAndFlag;
    nextNode = (PrtBestfitMemDynNode *)((UINTPTR)node + node->selfNode.sizeAndFlag);
    nextNode->selfNode.preNode = node->selfNode.preNode;
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node->selfNode.preNode);
    OsMemNodeSave(nextNode);
#endif
    OsMemClearNode(node);
}

/*
 * Description : split new node from allocNode, and merge remainder mem if necessary
 * Input       : pool      -- Pointer to memory pool
 *               allocNode -- the source node which the new node will split from it.
 *                            After pick up it's node info, change to point to the new node
 *               allocSize -- the size of new node
 * Output      : allocNode -- save new node addr
 */
STATIC INLINE VOID OsMemSplitNode(VOID *pool,
                                  PrtBestfitMemDynNode *allocNode, UINT32 allocSize)
{
    PrtBestfitMemDynNode *newFreeNode = NULL;
    PrtBestfitMemDynNode *nextNode = NULL;
    PRT_DL_LIST *listNodeHead = NULL;
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    newFreeNode = (PrtBestfitMemDynNode *)(VOID *)((UINT8 *)allocNode + allocSize);
    newFreeNode->selfNode.preNode = allocNode;
    newFreeNode->selfNode.sizeAndFlag = allocNode->selfNode.sizeAndFlag - allocSize;
    allocNode->selfNode.sizeAndFlag = allocSize;
    nextNode = OS_MEM_NEXT_NODE(newFreeNode);
    nextNode->selfNode.preNode = newFreeNode;
    if (!OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag)) {
        OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
        OsMemMergeNode(nextNode);
    }
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    else {
        OsMemNodeSave(nextNode);
    }
#endif
    listNodeHead = OS_MEM_HEAD(pool, newFreeNode->selfNode.sizeAndFlag);
    OS_CHECK_NULL_RETURN(listNodeHead);

    OsMemListAdd(listNodeHead, &newFreeNode->selfNode.freeNodeInfo, firstNode);
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(newFreeNode);
#endif
}

/*
 * Description : free the node from memory & if there are free node beside, merger them.
 *               at last update "listNodeHead' which saved all free node control head
 * Input       : node -- the node which need be freed
 *               pool -- Pointer to memory pool
 */
STATIC INLINE VOID OsMemFreeNode(PrtBestfitMemDynNode *node, PrtBestfitMemPoolInfo *pool)
{
    PrtBestfitMemDynNode *nextNode = NULL;
    PRT_DL_LIST *listNodeHead = NULL;
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    OS_MEM_REDUCE_USED(&pool->stat, OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag), OS_MEM_TASKID_GET(node));
    node->selfNode.sizeAndFlag = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
    if ((node->selfNode.preNode != NULL) &&
        !OS_MEM_NODE_GET_USED_FLAG(node->selfNode.preNode->selfNode.sizeAndFlag)) {
        PrtBestfitMemDynNode *preNode = node->selfNode.preNode;
        OsMemMergeNode(node);
        nextNode = OS_MEM_NEXT_NODE(preNode);
        if (!OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag)) {
            OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
            OsMemMergeNode(nextNode);
        }

        OsMemListDelete(&(preNode->selfNode.freeNodeInfo), firstNode);
        listNodeHead = OS_MEM_HEAD(pool, preNode->selfNode.sizeAndFlag);
        OS_CHECK_NULL_RETURN(listNodeHead);

        OsMemListAdd(listNodeHead, &preNode->selfNode.freeNodeInfo, firstNode);
    } else {
        nextNode = OS_MEM_NEXT_NODE(node);
        if (!OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag)) {
            OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
            OsMemMergeNode(nextNode);
        }

        listNodeHead = OS_MEM_HEAD(pool, node->selfNode.sizeAndFlag);
        OS_CHECK_NULL_RETURN(listNodeHead);

        OsMemListAdd(listNodeHead, &node->selfNode.freeNodeInfo, firstNode);
    }
}

#ifdef PRT_BESTFIT_CFG_MEM_DEBUG
/*
 * Description : check the result if pointer memory node belongs to pointer memory pool
 * Input       : pool -- Pointer to memory pool
 *               node -- the node which need be checked
 * Return      : PRT_OK or PRT_NOK
 */
STATIC INLINE BOOL OsMemIsNodeValid(const PrtBestfitMemDynNode *node, const PrtBestfitMemDynNode *startNode,
                                    const PrtBestfitMemDynNode *endNode,
                                    const UINT8 *startPool, const UINT8 *endPool)
{
    if (!OS_MEM_MIDDLE_ADDR(startNode, node, endNode)) {
        return FALSE;
    }

    if (OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        if (!OS_MEM_MAGIC_VALID(node->selfNode.freeNodeInfo.pstPrev)) {
            return FALSE;
        }
        return TRUE;
    }

    if (!OS_MEM_MIDDLE_ADDR_OPEN_END(startPool, node->selfNode.freeNodeInfo.pstPrev, endPool)) {
        return FALSE;
    }

    return TRUE;
}

STATIC INLINE UINT32 OsMemCheckUsedNode(const VOID *pool, const PrtBestfitMemDynNode *node)
{
    const PrtBestfitMemPoolInfo *poolInfo = (const PrtBestfitMemPoolInfo *)pool;
    const PrtBestfitMemDynNode *startNode = (const PrtBestfitMemDynNode *)OS_MEM_FIRST_NODE(pool);
    const PrtBestfitMemDynNode *endNode = (const PrtBestfitMemDynNode *)OS_MEM_END_NODE(pool, poolInfo->poolSize);
    const UINT8 *endPool = (const UINT8 *)pool + poolInfo->poolSize;
    const PrtBestfitMemDynNode *nextNode = NULL;
    if (!OsMemIsNodeValid(node, startNode, endNode, (UINT8 *)pool, endPool)) {
        return PRT_NOK;
    }

    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        return PRT_NOK;
    }

    nextNode = OS_MEM_NEXT_NODE(node);
    if (!OsMemIsNodeValid(nextNode, startNode, endNode, (UINT8 *)pool, endPool)) {
        return PRT_NOK;
    }

    if (nextNode->selfNode.preNode != node) {
        return PRT_NOK;
    }

    if (node != startNode) {
        if (!OsMemIsNodeValid(node->selfNode.preNode, startNode, endNode, (UINT8 *)pool, endPool)) {
            return PRT_NOK;
        }

        if (OS_MEM_NEXT_NODE(node->selfNode.preNode) != node) {
            return PRT_NOK;
        }
    }

    return PRT_OK;
}
#else
STATIC INLINE UINT32 OsMemCheckUsedNode(const VOID *pool, const PrtBestfitMemDynNode *node)
{
    (void)pool;

    if ((node == NULL) || !OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        return PRT_NOK;
    }

    return PRT_OK;
}
#endif

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_INTEGRITY_CHECK
PRT_BESTFIT_SEC_TEXT_MINOR STATIC INLINE UINT32 OsMemPoolDlinkcheck(const PrtBestfitMemPoolInfo *pool, PRT_DL_LIST listHead)
{
    if (((UINTPTR)listHead.pstPrev < (UINTPTR)(pool + 1)) ||
        ((UINTPTR)listHead.pstPrev >= ((UINTPTR)pool + pool->poolSize)) ||
        ((UINTPTR)listHead.pstNext < (UINTPTR)(pool + 1)) ||
        ((UINTPTR)listHead.pstNext >= ((UINTPTR)pool + pool->poolSize)) ||
        !IS_ALIGNED(listHead.pstPrev, sizeof(VOID *)) ||
        !IS_ALIGNED(listHead.pstNext, sizeof(VOID *))) {
        return PRT_NOK;
    }

    return PRT_OK;
}

/*
 * Description : show mem pool header info
 * Input       : pool --Pointer to memory pool
 */
PRT_BESTFIT_SEC_TEXT_MINOR VOID OsMemPoolHeadInfoPrint(const VOID *pool)
{
    const PrtBestfitMemPoolInfo *poolInfo = (const PrtBestfitMemPoolInfo *)pool;
    UINT32 dlinkNum;
    UINT32 flag = 0;
    const PrtBestfitMultipleDlinkHead *dlinkHead = NULL;

    if (!IS_ALIGNED(poolInfo, sizeof(VOID *))) {
        PRINT_ERR("wrong mem pool addr: %p, func:%s,line:%d\n", poolInfo, __FUNCTION__, __LINE__);
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("wrong mem pool addr: %p, func:%s,line:%d\n", poolInfo, __FUNCTION__, __LINE__);
#endif
        return;
    }

    dlinkHead = (const PrtBestfitMultipleDlinkHead *)(VOID *)(poolInfo + 1);
    for (dlinkNum = 0; dlinkNum < OS_MULTI_DLNK_NUM; dlinkNum++) {
        if (OsMemPoolDlinkcheck(pool, dlinkHead->listHead[dlinkNum])) {
            flag = 1;
            PRINT_ERR("DlinkHead[%u]: pstPrev:%p, pstNext:%p\n",
                      dlinkNum, dlinkHead->listHead[dlinkNum].pstPrev, dlinkHead->listHead[dlinkNum].pstNext);
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
            WriteExcInfoToBuf("DlinkHead[%u]: pstPrev:%p, pstNext:%p\n",
                              dlinkNum, dlinkHead->listHead[dlinkNum].pstPrev, dlinkHead->listHead[dlinkNum].pstNext);
#endif
        }
    }
    if (flag) {
        PRINTK("mem pool info: poolAddr:%p, poolSize:0x%x\n", poolInfo->pool, poolInfo->poolSize);
#ifdef PRT_BESTFIT_MEM_TASK_STAT
        PRINTK("mem pool info: poolWaterLine:0x%x, poolCurUsedSize:0x%x\n", poolInfo->stat.memTotalPeak,
               poolInfo->stat.memTotalUsed);
#endif

#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("mem pool info: poolAddr:%p, poolSize:0x%x\n", poolInfo->pool, poolInfo->poolSize);
#ifdef PRT_BESTFIT_MEM_TASK_STAT
        WriteExcInfoToBuf("mem pool info: poolWaterLine:0x%x, poolCurUsedSize:0x%x\n",
                          poolInfo->stat.memTotalPeak, poolInfo->stat.memTotalUsed);
#endif
#endif
    }
}

STATIC UINT32 OsMemIntegrityCheck(const VOID *pool, PrtBestfitMemDynNode **tmpNode, PrtBestfitMemDynNode **preNode)
{
    const PrtBestfitMemPoolInfo *poolInfo = (const PrtBestfitMemPoolInfo *)pool;
    const UINT8 *endPool = (const UINT8 *)pool + poolInfo->poolSize;

    OsMemPoolHeadInfoPrint(pool);

    *preNode = OS_MEM_FIRST_NODE(pool);
    for (*tmpNode = OS_MEM_FIRST_NODE(pool); *tmpNode < OS_MEM_END_NODE(pool, poolInfo->poolSize);
         *tmpNode = OS_MEM_NEXT_NODE(*tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG((*tmpNode)->selfNode.sizeAndFlag)) {
            if (!OS_MEM_MAGIC_VALID((*tmpNode)->selfNode.freeNodeInfo.pstPrev)) {
                PRINT_ERR("[%s], %d, memory check error!\n"
                          "memory used but magic num wrong, freeNodeInfo.pstPrev(magic num):%p \n",
                          __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev);
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
                WriteExcInfoToBuf("[%s], %d, memory check error!\n"
                                  "memory used but magic num wrong, freeNodeInfo.pstPrev(magic num):%p \n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev);
#endif
                return PRT_NOK;
            }
        } else { /* is free node, check free node range */
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pool, (*tmpNode)->selfNode.freeNodeInfo.pstPrev, endPool)) {
                PRINT_ERR("[%s], %d, memory check error!\n"
                          " freeNodeInfo.pstPrev:%p is out of legal mem range[%p, %p]\n",
                          __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev, pool, endPool);
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
                WriteExcInfoToBuf("[%s], %d, memory check error!\n"
                                  " freeNodeInfo.pstPrev:%p is out of legal mem range[%p, %p]\n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev, pool, endPool);
#endif
                return PRT_NOK;
            }
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pool, (*tmpNode)->selfNode.freeNodeInfo.pstNext, endPool)) {
                PRINT_ERR("[%s], %d, memory check error!\n"
                          " freeNodeInfo.pstNext:%p is out of legal mem range[%p, %p]\n",
                          __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstNext, pool, endPool);
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
                WriteExcInfoToBuf("[%s], %d, memory check error!\n"
                                  " freeNodeInfo.pstNext:%p is out of legal mem range[%p, %p]\n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstNext, pool, endPool);
#endif
                return PRT_NOK;
            }
        }

        *preNode = *tmpNode;
    }
    return PRT_OK;
}

STATIC VOID OsMemNodeInfo(const PrtBestfitMemDynNode *tmpNode,
                          const PrtBestfitMemDynNode *preNode)
{
    if (tmpNode == preNode) {
        PRINTK("\n the broken node is the first node\n");
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("\n the broken node is the first node\n");
#endif
    }
    PRINTK("\n broken node head: %p  %p  %p  0x%x, pre node head: %p  %p  %p  0x%x\n",
           tmpNode->selfNode.freeNodeInfo.pstPrev, tmpNode->selfNode.freeNodeInfo.pstNext,
           tmpNode->selfNode.preNode, tmpNode->selfNode.sizeAndFlag,
           preNode->selfNode.freeNodeInfo.pstPrev, preNode->selfNode.freeNodeInfo.pstNext,
           preNode->selfNode.preNode, preNode->selfNode.sizeAndFlag);

#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
    WriteExcInfoToBuf("\n broken node head: %p  %p  %p  0x%x, pre node head: %p  %p  %p  0x%x\n",
                      tmpNode->selfNode.freeNodeInfo.pstPrev, tmpNode->selfNode.freeNodeInfo.pstNext,
                      tmpNode->selfNode.preNode, tmpNode->selfNode.sizeAndFlag,
                      preNode->selfNode.freeNodeInfo.pstPrev, preNode->selfNode.freeNodeInfo.pstNext,
                      preNode->selfNode.preNode, preNode->selfNode.sizeAndFlag);
#endif
#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
    OsMemNodeBacktraceInfo(tmpNode, preNode);
#endif

    PRINTK("\n---------------------------------------------\n");
    UINTPTR dumpEnd = (((UINTPTR)tmpNode + NODEDUMPSIZE) > (UINTPTR)tmpNode) ?
                      ((UINTPTR)tmpNode + NODEDUMPSIZE) : (UINTPTR)(UINTPTR_MAX);
    UINT32 dumpSize = (UINTPTR)dumpEnd - (UINTPTR)tmpNode;
    PRINTK(" dump mem tmpNode:%p ~ %p\n", tmpNode, dumpEnd);
    OsDumpMemByte(dumpSize, (UINTPTR)tmpNode);
    PRINTK("\n---------------------------------------------\n");
    if (preNode != tmpNode) {
        PRINTK(" dump mem :%p ~ tmpNode:%p\n", ((UINTPTR)tmpNode - NODEDUMPSIZE), tmpNode);
        OsDumpMemByte(NODEDUMPSIZE, ((UINTPTR)tmpNode - NODEDUMPSIZE));
        PRINTK("\n---------------------------------------------\n");
    }
}

STATIC VOID OsMemIntegrityCheckError(const PrtBestfitMemDynNode *tmpNode,
                                     const PrtBestfitMemDynNode *preNode)
{
    PrtBestfitTaskCB *taskCB = NULL;
    UINT32 taskId;

    OsMemNodeInfo(tmpNode, preNode);

    taskId = OS_MEM_TASKID_GET(preNode);
    if (taskId >= g_taskMaxNum) {
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("Task ID %u in pre node is invalid!\n", taskId);
#endif
        PRT_Panic("Task ID %u in pre node is invalid!\n", taskId);
        return;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    if ((taskCB->taskStatus & OS_TASK_STATUS_UNUSED) ||
        (taskCB->taskEntry == NULL) || (taskCB->taskName == NULL)) {
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("\r\nTask ID %u in pre node is not created or deleted!\n", taskId);
#endif
        PRT_Panic("\r\nTask ID %u in pre node is not created!\n", taskId);
        return;
    }
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
    WriteExcInfoToBuf("cur node: %p\npre node: %p\npre node was allocated by task:%s\n",
                      tmpNode, preNode, taskCB->taskName);
#endif
    PRT_Panic("cur node: %p\npre node: %p\npre node was allocated by task:%s\n",
              tmpNode, preNode, taskCB->taskName);
    return;
}

/*
 * Description : memory pool integrity checking
 * Input       : pool --Pointer to memory pool
 * Return      : PRT_OK --memory pool integrate or PRT_NOK--memory pool impaired
 */
PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemIntegrityCheck(VOID *pool)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemDynNode *preNode = NULL;
    UINT32 intSave;

    if (pool == NULL) {
        return PRT_NOK;
    }

    MEM_LOCK(intSave);
    if (OsMemIntegrityCheck(pool, &tmpNode, &preNode)) {
        goto ERROR_OUT;
    }
    MEM_UNLOCK(intSave);
    return PRT_OK;

ERROR_OUT:
    OsMemIntegrityCheckError(tmpNode, preNode);
    MEM_UNLOCK(intSave);
    return PRT_NOK;
}

VOID OsMemIntegrityMultiCheck(VOID)
{
    if (PRT_BestfitMemIntegrityCheck(m_aucSysMem1) == PRT_OK) {
        PRINTK("system memcheck over, all passed!\n");
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("system memcheck over, all passed!\n");
#endif
    }
#ifdef PRT_BESTFIT_CFG_EXC_INTERACTION
    if (PRT_BestfitMemIntegrityCheck(m_aucSysMem0) == PRT_OK) {
        PRINTK("exc interaction memcheck over, all passed!\n");
#ifdef PRT_BESTFIT_CFG_SHELL_EXCINFO_DUMP
        WriteExcInfoToBuf("exc interaction memcheck over, all passed!\n");
#endif
    }
#endif
}
#else
PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemIntegrityCheck(VOID *pool)
{
    return PRT_OK;
}

VOID OsMemIntegrityMultiCheck(VOID)
{
}
#endif

STATIC INLINE VOID OsMemNodeDebugOperate(VOID *pool, PrtBestfitMemDynNode *allocNode, UINT32 size)
{
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    allocNode->selfNode.originSize = size;
#endif

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(allocNode);
#endif

#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(allocNode);
#endif
}

STATIC UINT32 OsMemInfoGet(const VOID *pool, PRT_BESTFIT_MEM_POOL_STATUS *poolStatus)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PrtBestfitMemDynNode *tmpNode = NULL;
    UINT32 totalUsedSize = 0;
    UINT32 totalFreeSize = 0;
    UINT32 maxFreeNodeSize = 0;
    UINT32 usedNodeNum = 0;
    UINT32 freeNodeNum = 0;

    tmpNode = (PrtBestfitMemDynNode *)OS_MEM_END_NODE(pool, poolInfo->poolSize);
    tmpNode = (PrtBestfitMemDynNode *)OS_MEM_ALIGN(tmpNode, OS_MEM_ALIGN_SIZE);
    if (!OS_MEM_MAGIC_VALID(tmpNode->selfNode.freeNodeInfo.pstPrev)) {
        PRINT_ERR("wrong mem pool addr: %p\n, line:%d", poolInfo, __LINE__);
        return PRT_NOK;
    }

    for (tmpNode = OS_MEM_FIRST_NODE(poolInfo); tmpNode <= OS_MEM_END_NODE(poolInfo, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (!OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            ++freeNodeNum;
            totalFreeSize += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
            if (maxFreeNodeSize < OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag)) {
                maxFreeNodeSize = OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
            }
        } else {
            ++usedNodeNum;
            totalUsedSize += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
        }
    }

    poolStatus->uwTotalUsedSize = totalUsedSize;
    poolStatus->uwTotalFreeSize = totalFreeSize;
    poolStatus->uwMaxFreeNodeSize = maxFreeNodeSize;
    poolStatus->uwUsedNodeNum = usedNodeNum;
    poolStatus->uwFreeNodeNum = freeNodeNum;
#ifdef PRT_BESTFIT_MEM_TASK_STAT
    poolStatus->uwUsageWaterLine = poolInfo->stat.memTotalPeak;
#endif

    return PRT_OK;
}

VOID OsMemInfoPrint(const VOID *pool)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PRT_BESTFIT_MEM_POOL_STATUS status = {0};

    if (OsMemInfoGet(pool, &status) == PRT_NOK) {
        return;
    }

#ifdef PRT_BESTFIT_MEM_TASK_STAT
    PRINTK("pool addr          pool size    used size     free size    "
           "max free node size   used node num     free node num      UsageWaterLine\n");
    PRINTK("---------------    --------     -------       --------     "
           "--------------       -------------      ------------      ------------\n");
    PRINTK("%-16p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x    0x%-13x\n",
           poolInfo->pool, poolInfo->poolSize, status.uwTotalUsedSize,
           status.uwTotalFreeSize, status.uwMaxFreeNodeSize, status.uwUsedNodeNum,
           status.uwFreeNodeNum, status.uwUsageWaterLine);

#else
    PRINTK("pool addr          pool size    used size     free size    "
           "max free node size   used node num     free node num\n");
    PRINTK("---------------    --------     -------       --------     "
           "--------------       -------------      ------------\n");
    PRINTK("%-16p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x\n",
           poolInfo->pool, poolInfo->poolSize, status.uwTotalUsedSize,
           status.uwTotalFreeSize, status.uwMaxFreeNodeSize, status.uwUsedNodeNum,
           status.uwFreeNodeNum);
#endif
}

STATIC INLINE VOID OsMemInfoAlert(const VOID *pool, UINT32 allocSize)
{
#ifdef PRT_BESTFIT_CFG_MEM_DEBUG
    PRINT_ERR("---------------------------------------------------"
              "--------------------------------------------------------\n");
    OsMemInfoPrint(pool);
    PRINT_ERR("[%s] No suitable free block, require free node size: 0x%x\n", __FUNCTION__, allocSize);
    PRINT_ERR("----------------------------------------------------"
              "-------------------------------------------------------\n");
#endif
}

/*
 * Description : Allocate node from Memory pool
 * Input       : pool  --- Pointer to memory pool
 *               size  --- Size of memory in bytes to allocate
 * Return      : Pointer to allocated memory
 */
STATIC VOID *OsMemAllocWithCheck(PrtBestfitMemPoolInfo *pool, UINT32 size)
{
    PrtBestfitMemDynNode *allocNode = NULL;
    UINT32 allocSize;

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_INTEGRITY_CHECK
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemDynNode *preNode = NULL;
#endif
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_INTEGRITY_CHECK
    if (OsMemIntegrityCheck(pool, &tmpNode, &preNode)) {
        OsMemIntegrityCheckError(tmpNode, preNode);
        return NULL;
    }
#endif

    allocSize = OS_MEM_ALIGN(size + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);
    allocNode = OsMemFindSuitableFreeBlock(pool, allocSize);
    if (allocNode == NULL) {
        OsMemInfoAlert(pool, allocSize);
        return NULL;
    }
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= allocNode->selfNode.sizeAndFlag) {
        OsMemSplitNode(pool, allocNode, allocSize);
    }
    OsMemListDelete(&allocNode->selfNode.freeNodeInfo, firstNode);
    OsMemSetMagicNumAndTaskID(allocNode);
    OS_MEM_NODE_SET_USED_FLAG(allocNode->selfNode.sizeAndFlag);
    OS_MEM_ADD_USED(&pool->stat, OS_MEM_NODE_GET_SIZE(allocNode->selfNode.sizeAndFlag),
                    OS_MEM_TASKID_GET(allocNode));
    OsMemNodeDebugOperate(pool, allocNode, size);
    return (allocNode + 1);
}

/*
 * Description : reAlloc a smaller memory node
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- the size of new node which will be alloced
 *               node      --- the node which will be realloced
 *               nodeSize  --- the size of old node
 * Output      : node      --- pointer to the new node after realloc
 */
STATIC INLINE VOID OsMemReAllocSmaller(PrtBestfitMemPoolInfo *pool, UINT32 allocSize, PrtBestfitMemDynNode *node, UINT32 nodeSize)
{
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= nodeSize) {
        node->selfNode.sizeAndFlag = nodeSize;
        OsMemSplitNode(pool, node, allocSize);
        OS_MEM_NODE_SET_USED_FLAG(node->selfNode.sizeAndFlag);
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
        OsMemNodeSave(node);
#endif

        OS_MEM_REDUCE_USED(&pool->stat, nodeSize - allocSize, OS_MEM_TASKID_GET(node));
    }
#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
}

/*
 * Description : reAlloc a Bigger memory node after merge node and nextNode
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- the size of new node which will be alloced
 *               node      --- the node which will be realloced
 *               nodeSize  --- the size of old node
 *               nextNode  --- pointer next node which will be merged
 * Output      : node      --- pointer to the new node after realloc
 */
STATIC INLINE VOID OsMemMergeNodeForReAllocBigger(PrtBestfitMemPoolInfo *pool, UINT32 allocSize, PrtBestfitMemDynNode *node,
                                                  UINT32 nodeSize, PrtBestfitMemDynNode *nextNode)
{
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    node->selfNode.sizeAndFlag = nodeSize;
    OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
    OsMemMergeNode(nextNode);
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= node->selfNode.sizeAndFlag) {
        OsMemSplitNode(pool, node, allocSize);
    }

    OS_MEM_ADD_USED(&pool->stat, node->selfNode.sizeAndFlag - nodeSize, OS_MEM_TASKID_GET(node));

    OS_MEM_NODE_SET_USED_FLAG(node->selfNode.sizeAndFlag);
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
#ifdef PRT_BESTFIT_CFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
}

STATIC UINT32 OsMemInit(VOID *pool, UINT32 size)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PrtBestfitMemDynNode *newNode = NULL;
    PrtBestfitMemDynNode *endNode = NULL;
    PRT_DL_LIST *listNodeHead = NULL;
    UINT32 poolSize = OsLmsMemInit(pool, size);
    if (poolSize == 0) {
        poolSize = size;
    }
    poolInfo->pool = pool;
    poolInfo->poolSize = poolSize;
    OsDLnkInitMultiHead(OS_MEM_HEAD_ADDR(pool));
    newNode = OS_MEM_FIRST_NODE(pool);
    newNode->selfNode.sizeAndFlag = (poolSize - (UINT32)((UINTPTR)newNode - (UINTPTR)pool) - OS_MEM_NODE_HEAD_SIZE);
    newNode->selfNode.preNode = (PrtBestfitMemDynNode *)OS_MEM_END_NODE(pool, poolSize);
    listNodeHead = OS_MEM_HEAD(pool, newNode->selfNode.sizeAndFlag);
    if (listNodeHead == NULL) {
        return PRT_NOK;
    }

    PRT_ListTailInsert(listNodeHead, &(newNode->selfNode.freeNodeInfo));
    endNode = (PrtBestfitMemDynNode *)OS_MEM_END_NODE(pool, poolSize);
    (VOID)memset_s(endNode, sizeof(*endNode), 0, sizeof(*endNode));
    endNode->selfNode.preNode = newNode;
    endNode->selfNode.sizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    OS_MEM_NODE_SET_USED_FLAG(endNode->selfNode.sizeAndFlag);
    OsMemSetMagicNumAndTaskID(endNode);
#ifdef PRT_BESTFIT_MEM_TASK_STAT
    UINT32 statSize = sizeof(poolInfo->stat);
    (VOID)memset_s(&poolInfo->stat, statSize, 0, statSize);
    poolInfo->stat.memTotalUsed = sizeof(PrtBestfitMemPoolInfo) + OS_MULTI_DLNK_HEAD_SIZE +
                                  OS_MEM_NODE_GET_SIZE(endNode->selfNode.sizeAndFlag);
    poolInfo->stat.memTotalPeak = poolInfo->stat.memTotalUsed;
#endif

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(newNode);
    OsMemNodeSave(endNode);
#endif

    return PRT_OK;
}

PRT_BESTFIT_SEC_TEXT_INIT UINT32 PRT_BestfitMemInit(VOID *pool, UINT32 size)
{
    UINT32 intSave;

    if ((pool == NULL) || (size < OS_MEM_MIN_POOL_SIZE)) {
        return PRT_NOK;
    }

    if (!IS_ALIGNED(size, OS_MEM_ALIGN_SIZE) || !IS_ALIGNED(pool, OS_MEM_ALIGN_SIZE)) {
        PRINT_WARN("pool [%p, %p) size 0x%x should be aligned with OS_MEM_ALIGN_SIZE\n",
                   pool, (UINTPTR)pool + size, size);
        size = OS_MEM_ALIGN(size, OS_MEM_ALIGN_SIZE) - OS_MEM_ALIGN_SIZE;
    }

    MEM_LOCK(intSave);
    if (OsMemMulPoolInit(pool, size)) {
        MEM_UNLOCK(intSave);
        return PRT_NOK;
    }

    if (OsMemInit(pool, size) != PRT_OK) {
        (VOID)OsMemMulPoolDeinit(pool);
        MEM_UNLOCK(intSave);
        return PRT_NOK;
    }

    OsSlabMemInit(pool, size);
    MEM_UNLOCK(intSave);

    PRT_TRACE(MEM_INFO_REQ, pool);
    return PRT_OK;
}

PRT_BESTFIT_SEC_TEXT VOID *PRT_BestfitMemAlloc(VOID *pool, UINT32 size)
{
    VOID *ptr = NULL;
    UINT32 intSave;

    if ((pool == NULL) || (size == 0)) {
        return NULL;
    }

    if (g_MALLOC_HOOK != NULL) {
        g_MALLOC_HOOK();
    }

    MEM_LOCK(intSave);
    do {
        if (OS_MEM_NODE_GET_USED_FLAG(size) || OS_MEM_NODE_GET_ALIGNED_FLAG(size)) {
            break;
        }

        ptr = OsSlabMemAlloc(pool, size);
        if (ptr == NULL) {
            ptr = OsMemAllocWithCheck(pool, size);
        }
    } while (0);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    OsMemRecordMalloc(ptr, size);
#endif
    OsLmsSetAfterMalloc(ptr);

    MEM_UNLOCK(intSave);

    PRT_TRACE(MEM_ALLOC, pool, (UINTPTR)ptr, size);
    return ptr;
}

PRT_BESTFIT_SEC_TEXT VOID *PRT_BestfitMemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary)
{
    UINT32 useSize;
    UINT32 gapSize;
    VOID *ptr = NULL;
    VOID *alignedPtr = NULL;
    PrtBestfitMemDynNode *allocNode = NULL;
    UINT32 intSave;

    if ((pool == NULL) || (size == 0) || (boundary == 0) || !IS_POW_TWO(boundary) ||
        !IS_ALIGNED(boundary, sizeof(VOID *))) {
        return NULL;
    }

    MEM_LOCK(intSave);
    do {
        /*
         * sizeof(gapSize) bytes stores offset between alignedPtr and ptr,
         * the ptr has been OS_MEM_ALIGN_SIZE(4 or 8) aligned, so maximum
         * offset between alignedPtr and ptr is boundary - OS_MEM_ALIGN_SIZE
         */
        if ((boundary - sizeof(gapSize)) > ((UINT32)(-1) - size)) {
            break;
        }

        useSize = (size + boundary) - sizeof(gapSize);
        if (OS_MEM_NODE_GET_USED_FLAG(useSize) || OS_MEM_NODE_GET_ALIGNED_FLAG(useSize)) {
            break;
        }

        ptr = OsMemAllocWithCheck(pool, useSize);

        alignedPtr = (VOID *)OS_MEM_ALIGN(ptr, boundary);
        if (ptr == alignedPtr) {
            break;
        }

        /* store gapSize in address (ptr -4), it will be checked while free */
        gapSize = (UINT32)((UINTPTR)alignedPtr - (UINTPTR)ptr);
        allocNode = (PrtBestfitMemDynNode *)ptr - 1;
        OS_MEM_NODE_SET_ALIGNED_FLAG(allocNode->selfNode.sizeAndFlag);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        allocNode->selfNode.originSize = size;
#endif
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
        OsMemNodeSaveWithGapSize(allocNode, gapSize);
#endif

        OS_MEM_NODE_SET_ALIGNED_FLAG(gapSize);
        *(UINT32 *)((UINTPTR)alignedPtr - sizeof(gapSize)) = gapSize;

        ptr = alignedPtr;
    } while (0);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    OsMemRecordMalloc(ptr, size);
#endif
    OsLmsSetAfterMalloc(ptr);

    MEM_UNLOCK(intSave);

    PRT_TRACE(MEM_ALLOC_ALIGN, pool, (UINTPTR)ptr, size, boundary);
    return ptr;
}

PRT_BESTFIT_SEC_TEXT STATIC INLINE UINT32 OsDoMemFree(VOID *pool, const VOID *ptr, PrtBestfitMemDynNode *node)
{
    UINT32 ret = OsMemCheckUsedNode(pool, node);
    if (ret == PRT_OK) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(ptr, node->selfNode.originSize);
#endif
        OsMemFreeNode(node, pool);
    }
    return ret;
}

UINT32 OsBestfitHeapFree(VOID *pool, VOID *ptr)
{
    UINT32 ret = PRT_NOK;
    UINT32 gapSize;
    PrtBestfitMemDynNode *node = NULL;

    do {
        gapSize = *(UINT32 *)((UINTPTR)ptr - sizeof(UINT32));
        if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize) && OS_MEM_NODE_GET_USED_FLAG(gapSize)) {
            PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
            goto OUT;
        }

        node = (PrtBestfitMemDynNode *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);

        if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize)) {
            gapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(gapSize);
            if ((gapSize & (OS_MEM_ALIGN_SIZE - 1)) || (gapSize > ((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE))) {
                PRINT_ERR("illegal gapSize: 0x%x\n", gapSize);
                break;
            }
            node = (PrtBestfitMemDynNode *)((UINTPTR)ptr - gapSize - OS_MEM_NODE_HEAD_SIZE);
        }
#ifndef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
        ret = OsDoMemFree(pool, ptr, node);
#endif
    } while (0);
#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    ret = OsMemBackupCheckAndRetore(pool, ptr, node);
    if (!ret) {
        ret = OsDoMemFree(pool, ptr, node);
    }
#endif

OUT:
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    if (ret == PRT_NOK) {
        OsMemRecordFree(ptr, 0);
    }
#endif
    return ret;
}

PRT_BESTFIT_SEC_TEXT UINT32 PRT_BestfitMemFree(VOID *pool, VOID *ptr)
{
    UINT32 ret;
    UINT32 intSave;

    if ((pool == NULL) || (ptr == NULL) ||
        !IS_ALIGNED(pool, sizeof(VOID *)) || !IS_ALIGNED(ptr, sizeof(VOID *))) {
        return PRT_NOK;
    }

    MEM_LOCK(intSave);

    if (OsSlabMemFree(pool, ptr)) {
        ret = PRT_OK;
        goto OUT;
    }

    ret = OsBestfitHeapFree(pool, ptr);
OUT:
    OsLmsSetAfterFree(ptr);
    MEM_UNLOCK(intSave);

    PRT_TRACE(MEM_FREE, pool, (UINTPTR)ptr);
    return ret;
}

STATIC VOID *OsGetRealPtr(const VOID *pool, VOID *ptr)
{
    VOID *realPtr = ptr;
    UINT32 gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(UINT32)));

    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize) && OS_MEM_NODE_GET_USED_FLAG(gapSize)) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(ptr, 0);
#endif
        PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
        return NULL;
    }
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize)) {
        gapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(gapSize);
        if ((gapSize & (OS_MEM_ALIGN_SIZE - 1)) ||
            (gapSize > ((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE - (UINTPTR)pool))) {
            PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
            OsMemRecordFree(ptr, 0);
#endif
            return NULL;
        }
        realPtr = (VOID *)((UINTPTR)ptr - (UINTPTR)gapSize);
    }
    return realPtr;
}

STATIC VOID *OsMemRealloc(VOID *pool, VOID *ptr, UINT32 size)
{
    PrtBestfitMemDynNode *node = NULL;
    PrtBestfitMemDynNode *nextNode = NULL;
    VOID *tmpPtr = NULL;
    VOID *realPtr = NULL;
    UINT32 nodeSize;
    UINT32 allocSize = OS_MEM_ALIGN(size + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
    const VOID *originPtr = ptr;
#endif

    realPtr = OsGetRealPtr(pool, ptr);
    if (realPtr == NULL) {
        return NULL;
    }

    node = (PrtBestfitMemDynNode *)((UINTPTR)realPtr - OS_MEM_NODE_HEAD_SIZE);
    if (OsMemCheckUsedNode(pool, node) != PRT_OK) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(originPtr, 0);
#endif
        return NULL;
    }

    nodeSize = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
    if (nodeSize >= allocSize) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(originPtr, node->selfNode.originSize);
#endif
        OsMemReAllocSmaller(pool, allocSize, node, nodeSize);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemReallocNodeRecord(node, size, ptr);
#endif
        return (VOID *)ptr;
    }

    nextNode = OS_MEM_NEXT_NODE(node);
    if (!OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag) &&
        ((nextNode->selfNode.sizeAndFlag + nodeSize) >= allocSize)) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(originPtr, node->selfNode.originSize);
#endif
        OsMemMergeNodeForReAllocBigger(pool, allocSize, node, nodeSize, nextNode);
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemReallocNodeRecord(node, size, ptr);
#endif
        return (VOID *)ptr;
    }

    tmpPtr = OsMemAllocWithCheck(pool, size);
    if (tmpPtr != NULL) {
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordMalloc(tmpPtr, size);
#endif
        UINT32 gapSize = (UINT32)((UINTPTR)ptr - (UINTPTR)realPtr);
        if (memcpy_s(tmpPtr, size, ptr, (nodeSize - OS_MEM_NODE_HEAD_SIZE - gapSize)) != EOK) {
            (VOID)OsBestfitHeapFree((VOID *)pool, (VOID *)tmpPtr);
            return NULL;
        }
#ifdef PRT_BESTFIT_CFG_MEM_RECORDINFO
        OsMemRecordFree(originPtr, node->selfNode.originSize);
#endif
        OsMemFreeNode(node, pool);
    }
    return tmpPtr;
}

PRT_BESTFIT_SEC_TEXT_MINOR VOID *PRT_BestfitMemRealloc(VOID *pool, VOID *ptr, UINT32 size)
{
    UINT32 intSave;
    VOID *newPtr = NULL;
    BOOL isSlabMem = FALSE;

    if (OS_MEM_NODE_GET_USED_FLAG(size) || OS_MEM_NODE_GET_ALIGNED_FLAG(size) || (pool == NULL)) {
        return NULL;
    }

    if (ptr == NULL) {
        newPtr = PRT_BestfitMemAlloc(pool, size);
        goto OUT;
    }

    if (size == 0) {
        (VOID)PRT_BestfitMemFree(pool, ptr);
        goto OUT;
    }

    MEM_LOCK(intSave);

    newPtr = OsMemReallocSlab(pool, ptr, &isSlabMem, size);
    if (isSlabMem == TRUE) {
        goto OUT_UNLOCK;
    }

    newPtr = OsMemRealloc(pool, ptr, size);

OUT_UNLOCK:
    MEM_UNLOCK(intSave);
OUT:

    PRT_TRACE(MEM_REALLOC, pool, (UINTPTR)ptr, size);
    return newPtr;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemTotalUsedGet(VOID *pool)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 memUsed = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return PRT_NOK;
    }

    MEM_LOCK(intSave);

    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= OS_MEM_END_NODE(pool, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            memUsed += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
        }
    }

    MEM_UNLOCK(intSave);

    return memUsed;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemUsedBlksGet(VOID *pool)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 blkNums = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return PRT_NOK;
    }

    MEM_LOCK(intSave);

    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= OS_MEM_END_NODE(pool, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            blkNums++;
        }
    }

    MEM_UNLOCK(intSave);

    return blkNums;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemTaskIdGet(const VOID *ptr)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)(VOID *)m_aucSysMem1;
    UINT32 intSave;
#ifdef PRT_BESTFIT_CFG_EXC_INTERACTION
    if (ptr < (VOID *)m_aucSysMem1) {
        poolInfo = (PrtBestfitMemPoolInfo *)(VOID *)m_aucSysMem0;
    }
#endif

    if ((ptr == NULL) ||
        (ptr < (VOID *)OS_MEM_FIRST_NODE(poolInfo)) ||
        (ptr > (VOID *)OS_MEM_END_NODE(poolInfo, poolInfo->poolSize))) {
        PRINT_ERR("input ptr %p is out of system memory range[%p, %p]\n", ptr, OS_MEM_FIRST_NODE(poolInfo),
                  OS_MEM_END_NODE(poolInfo, poolInfo->poolSize));
        return OS_INVALID;
    }

    MEM_LOCK(intSave);

    for (tmpNode = OS_MEM_FIRST_NODE(poolInfo); tmpNode <= OS_MEM_END_NODE(poolInfo, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if ((UINTPTR)ptr < (UINTPTR)tmpNode) {
            if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.preNode->selfNode.sizeAndFlag)) {
                MEM_UNLOCK(intSave);
                return tmpNode->selfNode.preNode->selfNode.taskId;
            } else {
                MEM_UNLOCK(intSave);
                PRINT_ERR("input ptr %p is belong to a free mem node\n", ptr);
                return OS_INVALID;
            }
        }
    }

    MEM_UNLOCK(intSave);
    return OS_INVALID;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemFreeBlksGet(VOID *pool)
{
    PrtBestfitMemDynNode *tmpNode = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 blkNums = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return PRT_NOK;
    }

    MEM_LOCK(intSave);

    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= OS_MEM_END_NODE(pool, poolInfo->poolSize);
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (!OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            blkNums++;
        }
    }

    MEM_UNLOCK(intSave);

    return blkNums;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINTPTR PRT_BestfitMemLastUsedGet(VOID *pool)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    PrtBestfitMemDynNode *node = NULL;

    if (pool == NULL) {
        return PRT_NOK;
    }

    node = OS_MEM_END_NODE(pool, poolInfo->poolSize)->selfNode.preNode;
    if (OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        return (UINTPTR)((CHAR *)node + OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag) + sizeof(PrtBestfitMemDynNode));
    } else {
        return (UINTPTR)((CHAR *)node + sizeof(PrtBestfitMemDynNode));
    }
}

/*
 * Description : reset "end node"
 * Input       : pool    --- Pointer to memory pool
 *               preAddr --- Pointer to the pre Pointer of end node
 * Output      : endNode --- pointer to "end node"
 * Return      : the number of free node
 */
PRT_BESTFIT_SEC_TEXT_MINOR VOID OsMemResetEndNode(VOID *pool, UINTPTR preAddr)
{
    PrtBestfitMemDynNode *endNode = (PrtBestfitMemDynNode *)OS_MEM_END_NODE(pool, ((PrtBestfitMemPoolInfo *)pool)->poolSize);
    endNode->selfNode.sizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    if (preAddr != 0) {
        endNode->selfNode.preNode = (PrtBestfitMemDynNode *)(preAddr - sizeof(PrtBestfitMemDynNode));
    }
    OS_MEM_NODE_SET_USED_FLAG(endNode->selfNode.sizeAndFlag);
    OsMemSetMagicNumAndTaskID(endNode);

#ifdef PRT_BESTFIT_CFG_MEM_HEAD_BACKUP
    OsMemNodeSave(endNode);
#endif
}

UINT32 PRT_BestfitMemPoolSizeGet(const VOID *pool)
{
    if (pool == NULL) {
        return PRT_NOK;
    }
    return ((PrtBestfitMemPoolInfo *)pool)->poolSize;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemInfoGet(VOID *pool, PRT_BESTFIT_MEM_POOL_STATUS *poolStatus)
{
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 ret;
    UINT32 intSave;

    if (poolStatus == NULL) {
        PRINT_ERR("can't use NULL addr to save info\n");
        return PRT_NOK;
    }

    if ((poolInfo == NULL) || ((UINTPTR)pool != (UINTPTR)poolInfo->pool)) {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", poolInfo, __LINE__);
        return PRT_NOK;
    }

    MEM_LOCK(intSave);
    ret = OsMemInfoGet(poolInfo, poolStatus);
    MEM_UNLOCK(intSave);

    return ret;
}

STATIC INLINE VOID OsShowFreeNode(UINT32 index, UINT32 length, const UINT32 *countNum)
{
    UINT32 count = 0;
    PRINTK("\n    block size:  ");
    while (count < length) {
        PRINTK("2^%-5u", (index + OS_MIN_MULTI_DLNK_LOG2 + count));
        count++;
    }
    PRINTK("\n    node number: ");
    count = 0;
    while (count < length) {
        PRINTK("  %-5u", countNum[count + index]);
        count++;
    }
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemFreeNodeShow(VOID *pool)
{
    PRT_DL_LIST *listNodeHead = NULL;
    PrtBestfitMultipleDlinkHead *headAddr = (PrtBestfitMultipleDlinkHead *)((UINTPTR)pool + sizeof(PrtBestfitMemPoolInfo));
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT32 linkHeadIndex;
    UINT32 countNum[OS_MULTI_DLNK_NUM] = { 0 };
    UINT32 intSave;

    if ((poolInfo == NULL) || ((UINTPTR)pool != (UINTPTR)poolInfo->pool)) {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", poolInfo, __LINE__);
        return PRT_NOK;
    }

    PRINTK("\n   ************************ left free node number**********************");
    MEM_LOCK(intSave);

    for (linkHeadIndex = 0; linkHeadIndex <= (OS_MULTI_DLNK_NUM - 1);
         linkHeadIndex++) {
        listNodeHead = headAddr->listHead[linkHeadIndex].pstNext;
        while (listNodeHead != &(headAddr->listHead[linkHeadIndex])) {
            listNodeHead = listNodeHead->pstNext;
            countNum[linkHeadIndex]++;
        }
    }

    linkHeadIndex = 0;
    while (linkHeadIndex < OS_MULTI_DLNK_NUM) {
        if (linkHeadIndex + COLUMN_NUM < OS_MULTI_DLNK_NUM) {
            OsShowFreeNode(linkHeadIndex, COLUMN_NUM, countNum);
            linkHeadIndex += COLUMN_NUM;
        } else {
            OsShowFreeNode(linkHeadIndex, (OS_MULTI_DLNK_NUM - 1 - linkHeadIndex), countNum);
            break;
        }
    }

    MEM_UNLOCK(intSave);
    PRINTK("\n   ********************************************************************\n\n");

    return PRT_OK;
}

#ifdef PRT_BESTFIT_CFG_BASE_MEM_NODE_SIZE_CHECK

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemNodeSizeCheck(VOID *pool, VOID *ptr, UINT32 *totalSize, UINT32 *availSize)
{
    const VOID *head = NULL;
    PrtBestfitMemPoolInfo *poolInfo = (PrtBestfitMemPoolInfo *)pool;
    UINT8 *endPool = NULL;

    if (g_memCheckLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_DISABLE) {
        return PRT_BESTFIT_ERRNO_MEMCHECK_DISABLED;
    }

    if ((pool == NULL) || (ptr == NULL) || (totalSize == NULL) || (availSize == NULL)) {
        return PRT_BESTFIT_ERRNO_MEMCHECK_PARA_NULL;
    }

    endPool = (UINT8 *)pool + poolInfo->poolSize;
    if (!(OS_MEM_MIDDLE_ADDR_OPEN_END(pool, ptr, endPool))) {
        return PRT_BESTFIT_ERRNO_MEMCHECK_OUTSIDE;
    }

    if (g_memCheckLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_HIGH) {
        head = OsMemFindNodeCtrl(pool, ptr);
        if ((head == NULL) ||
            (OS_MEM_NODE_GET_SIZE(((PrtBestfitMemDynNode *)head)->selfNode.sizeAndFlag) < ((UINTPTR)ptr - (UINTPTR)head))) {
            return PRT_BESTFIT_ERRNO_MEMCHECK_NO_HEAD;
        }
        *totalSize = OS_MEM_NODE_GET_SIZE(((PrtBestfitMemDynNode *)head)->selfNode.sizeAndFlag - sizeof(PrtBestfitMemDynNode));
        *availSize = OS_MEM_NODE_GET_SIZE(((PrtBestfitMemDynNode *)head)->selfNode.sizeAndFlag - ((UINTPTR)ptr -
                                          (UINTPTR)head));
        return PRT_OK;
    }
    if (g_memCheckLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_LOW) {
        if (ptr != (VOID *)OS_MEM_ALIGN(ptr, OS_MEM_ALIGN_SIZE)) {
            return PRT_BESTFIT_ERRNO_MEMCHECK_NO_HEAD;
        }
        head = (const VOID *)((UINTPTR)ptr - sizeof(PrtBestfitMemDynNode));
        if (OS_MEM_MAGIC_VALID(((PrtBestfitMemDynNode *)head)->selfNode.freeNodeInfo.pstPrev)) {
            *totalSize = OS_MEM_NODE_GET_SIZE(((PrtBestfitMemDynNode *)head)->selfNode.sizeAndFlag - sizeof(PrtBestfitMemDynNode));
            *availSize = OS_MEM_NODE_GET_SIZE(((PrtBestfitMemDynNode *)head)->selfNode.sizeAndFlag - sizeof(PrtBestfitMemDynNode));
            return PRT_OK;
        } else {
            return PRT_BESTFIT_ERRNO_MEMCHECK_NO_HEAD;
        }
    }

    return PRT_BESTFIT_ERRNO_MEMCHECK_WRONG_LEVEL;
}

/*
 * Description : get a pool's memCtrl
 * Input       : ptr -- point to source ptr
 * Return      : search forward for ptr's memCtrl or "NULL"
 * attention : this func couldn't ensure the return memCtrl belongs to ptr it just find forward the most nearly one
 */
PRT_BESTFIT_SEC_TEXT_MINOR const VOID *OsMemFindNodeCtrl(const VOID *pool, const VOID *ptr)
{
    const VOID *head = ptr;

    if (ptr == NULL) {
        return NULL;
    }

    head = (const VOID *)OS_MEM_ALIGN(head, OS_MEM_ALIGN_SIZE);
    while (!OS_MEM_MAGIC_VALID(((PrtBestfitMemDynNode *)head)->selfNode.freeNodeInfo.pstPrev)) {
        head = (const VOID *)((UINT8 *)head - sizeof(CHAR *));
        if (head <= pool) {
            return NULL;
        }
    }
    return head;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT32 PRT_BestfitMemCheckLevelSet(UINT8 checkLevel)
{
    if (checkLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_LOW) {
        PRINTK("%s: PRT_BESTFIT_MEM_CHECK_LEVEL_LOW \n", __FUNCTION__);
    } else if (checkLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_HIGH) {
        PRINTK("%s: PRT_BESTFIT_MEM_CHECK_LEVEL_HIGH \n", __FUNCTION__);
    } else if (checkLevel == PRT_BESTFIT_MEM_CHECK_LEVEL_DISABLE) {
        PRINTK("%s: PRT_BESTFIT_MEM_CHECK_LEVEL_DISABLE \n", __FUNCTION__);
    } else {
        PRINTK("%s: wrong param, setting failed !! \n", __FUNCTION__);
        return PRT_BESTFIT_ERRNO_MEMCHECK_WRONG_LEVEL;
    }
    g_memCheckLevel = checkLevel;
    return PRT_OK;
}

PRT_BESTFIT_SEC_TEXT_MINOR UINT8 PRT_BestfitMemCheckLevelGet(VOID)
{
    return g_memCheckLevel;
}


UINT32 OsMemSysNodeCheck(VOID *dstAddr, VOID *srcAddr, UINT32 nodeLength, UINT8 pos)
{
    UINT32 ret;
    UINT32 totalSize = 0;
    UINT32 availSize = 0;
    UINT8 *pool = m_aucSysMem1;
#ifdef PRT_BESTFIT_CFG_EXC_INTERACTION
    if ((UINTPTR)dstAddr < ((UINTPTR)m_aucSysMem0 + g_excInteractMemSize)) {
        pool = m_aucSysMem0;
    }
#endif
    if (pos == 0) { /* if this func was called by memset */
        ret = PRT_BestfitMemNodeSizeCheck(pool, dstAddr, &totalSize, &availSize);
        if ((ret == PRT_OK) && (nodeLength > availSize)) {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memset: dst inode availSize is not enough"
                      " availSize = 0x%x, memset length = 0x%x\n", availSize, nodeLength);
            OsBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return PRT_NOK;
        }
    } else if (pos == 1) { /* if this func was called by memcpy */
        ret = PRT_BestfitMemNodeSizeCheck(pool, dstAddr, &totalSize, &availSize);
        if ((ret == PRT_OK) && (nodeLength > availSize)) {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memcpy: dst inode availSize is not enough"
                      " availSize = 0x%x, memcpy length = 0x%x\n", availSize, nodeLength);
            OsBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return PRT_NOK;
        }
#ifdef PRT_BESTFIT_CFG_EXC_INTERACTION
        if ((UINTPTR)srcAddr < ((UINTPTR)m_aucSysMem0 + g_excInteractMemSize)) {
            pool = m_aucSysMem0;
        } else {
            pool = m_aucSysMem1;
        }
#endif
        ret = PRT_BestfitMemNodeSizeCheck(pool, srcAddr, &totalSize, &availSize);
        if ((ret == PRT_OK) && (nodeLength > availSize)) {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memcpy: src inode availSize is not enough"
                      " availSize = 0x%x, memcpy length = 0x%x\n",
                      availSize, nodeLength);
            OsBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return PRT_NOK;
        }
    }
    return PRT_OK;
}
#endif /* PRT_BESTFIT_CFG_BASE_MEM_NODE_SIZE_CHECK */

#ifdef PRT_BESTFIT_CFG_MEM_MUL_MODULE
STATIC INLINE UINT32 OsMemModCheck(UINT32 moduleId)
{
    if (moduleId > MEM_MODULE_MAX) {
        PRINT_ERR("error module ID input!\n");
        return PRT_NOK;
    }
    return PRT_OK;
}

STATIC INLINE VOID *OsMemPtrToNode(VOID *ptr)
{
    UINT32 gapSize;

    if ((UINTPTR)ptr & (OS_MEM_ALIGN_SIZE - 1)) {
        PRINT_ERR("[%s:%d]ptr:%p not align by 4byte\n", __FUNCTION__, __LINE__, ptr);
        return NULL;
    }

    gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(UINT32)));
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize) && OS_MEM_NODE_GET_USED_FLAG(gapSize)) {
        PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
        return NULL;
    }
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize)) {
        gapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(gapSize);
        if ((gapSize & (OS_MEM_ALIGN_SIZE - 1)) || (gapSize > ((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE))) {
            PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
            return NULL;
        }

        ptr = (VOID *)((UINTPTR)ptr - gapSize);
    }

    return (VOID *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
}

STATIC INLINE UINT32 OsMemNodeSizeGet(VOID *ptr)
{
    PrtBestfitMemDynNode *node = (PrtBestfitMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return 0;
    }

    return OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
}

VOID *PRT_BestfitMemMalloc(VOID *pool, UINT32 size, UINT32 moduleId)
{
    UINT32 intSave;
    VOID *ptr = NULL;
    VOID *node = NULL;
    if (OsMemModCheck(moduleId) == PRT_NOK) {
        return NULL;
    }
    ptr = PRT_BestfitMemAlloc(pool, size);
    if (ptr != NULL) {
        MEM_LOCK(intSave);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(ptr);
        node = OsMemPtrToNode(ptr);
        if (node != NULL) {
            OS_MEM_MODID_SET(node, moduleId);
        }
        MEM_UNLOCK(intSave);
    }
    return ptr;
}

VOID *PRT_BestfitMemMallocAlign(VOID *pool, UINT32 size, UINT32 boundary, UINT32 moduleId)
{
    UINT32 intSave;
    VOID *ptr = NULL;
    VOID *node = NULL;
    if (OsMemModCheck(moduleId) == PRT_NOK) {
        return NULL;
    }
    ptr = PRT_BestfitMemAllocAlign(pool, size, boundary);
    if (ptr != NULL) {
        MEM_LOCK(intSave);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(ptr);
        node = OsMemPtrToNode(ptr);
        if (node != NULL) {
            OS_MEM_MODID_SET(node, moduleId);
        }
        MEM_UNLOCK(intSave);
    }
    return ptr;
}

UINT32 PRT_BestfitMemMfree(VOID *pool, VOID *ptr, UINT32 moduleId)
{
    UINT32 intSave;
    UINT32 ret;
    UINT32 size;
    PrtBestfitMemDynNode *node = NULL;

    if ((OsMemModCheck(moduleId) == PRT_NOK) || (ptr == NULL) || (pool == NULL)) {
        return PRT_NOK;
    }

    node = (PrtBestfitMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return PRT_NOK;
    }

    size = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);

    if (moduleId != OS_MEM_MODID_GET(node)) {
        PRINT_ERR("node[%p] alloced in module %lu, but free in module %u\n node's taskId: 0x%x\n",
                  ptr, OS_MEM_MODID_GET(node), moduleId, OS_MEM_TASKID_GET(node));
        moduleId = OS_MEM_MODID_GET(node);
    }

    ret = PRT_BestfitMemFree(pool, ptr);
    if (ret == PRT_OK) {
        MEM_LOCK(intSave);
        g_moduleMemUsedSize[moduleId] -= size;
        MEM_UNLOCK(intSave);
    }
    return ret;
}

VOID *PRT_BestfitMemMrealloc(VOID *pool, VOID *ptr, UINT32 size, UINT32 moduleId)
{
    VOID *newPtr = NULL;
    UINT32 oldNodeSize;
    UINT32 intSave;
    PrtBestfitMemDynNode *node = NULL;
    UINT32 oldModuleId = moduleId;

    if ((OsMemModCheck(moduleId) == PRT_NOK) || (pool == NULL)) {
        return NULL;
    }

    if (ptr == NULL) {
        return PRT_BestfitMemMalloc(pool, size, moduleId);
    }

    node = (PrtBestfitMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return NULL;
    }

    if (moduleId != OS_MEM_MODID_GET(node)) {
        PRINT_ERR("a node[%p] alloced in module %lu, but realloc in module %u\n node's taskId: %lu\n",
                  ptr, OS_MEM_MODID_GET(node), moduleId, OS_MEM_TASKID_GET(node));
        oldModuleId = OS_MEM_MODID_GET(node);
    }

    if (size == 0) {
        (VOID)PRT_BestfitMemMfree(pool, ptr, oldModuleId);
        return NULL;
    }

    oldNodeSize = OsMemNodeSizeGet(ptr);
    newPtr = PRT_BestfitMemRealloc(pool, ptr, size);
    if (newPtr != NULL) {
        MEM_LOCK(intSave);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(newPtr);
        g_moduleMemUsedSize[oldModuleId] -= oldNodeSize;
        node = (PrtBestfitMemDynNode *)OsMemPtrToNode(newPtr);
        OS_MEM_MODID_SET(node, moduleId);
        MEM_UNLOCK(intSave);
    }
    return newPtr;
}

UINT32 PRT_BestfitMemMusedGet(UINT32 moduleId)
{
    if (OsMemModCheck(moduleId) == PRT_NOK) {
        return OS_NULL_INT;
    }
    return g_moduleMemUsedSize[moduleId];
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
