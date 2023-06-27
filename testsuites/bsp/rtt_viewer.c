#include "rtt_viewer.h"
#include "prt_typedef.h"
#include "prt_mem.h"
#include "securec.h"

TagRttViewerCb _SEGGER_RTT;

/*
 * 描述: rtt viewer加锁
 */
static unsigned int RttViewerLock()
{
    unsigned int lockState;
    OS_EMBED_ASM("mrs   %0, basepri  \n\t"
                 "mov   r1, %1       \n\t"
                 "msr   basepri, r1  \n\t"
                 : "=r" (lockState) \
                 : "i"(RTT_VIEWER_MAX_INT_PRI)
                 : "r1", "cc"
                 );

    return lockState;
}

/*
 * 描述: rtt viewer释放锁
 */
static void RttViewerUnlock(unsigned int lockSave)
{
    OS_EMBED_ASM("msr   basepri, %0  \n\t"
                 :
                 : "r" (lockSave)
                 :
                 );
}

/*
 * 描述: rtt viewer初始化
 */
void RttViewerInit(void)
{
    unsigned int idx;
    unsigned int size;
    unsigned char *wbuffer;
    unsigned char *rbuffer;

    g_rttViewerCb.wbMaxNum = RTT_VIEWER_WB_MAX_NUM;
    g_rttViewerCb.rbMaxNum = RTT_VIEWER_RB_MAX_NUM;

    /* rtt viewer写缓冲区资源申请 */
    size = RTT_VIEWER_WB_MAX_NUM * RTT_VIEWER_WB_BUFFER_SIZE;
    wbuffer = (unsigned char *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, size);
    if ((wbuffer == NULL) || (memset_s(wbuffer, size, 0, size) != EOK)) {
        return;
    }

    /* rtt viewer写缓冲区初始化 */
    for (idx = 0; idx < RTT_VIEWER_WB_MAX_NUM; idx++) {
        g_rttViewerCb.aUp[idx].sName = "Terminal";
        g_rttViewerCb.aUp[idx].pBuffer = &wbuffer[idx * RTT_VIEWER_WB_BUFFER_SIZE];
        g_rttViewerCb.aUp[idx].size = RTT_VIEWER_WB_BUFFER_SIZE;
        g_rttViewerCb.aUp[idx].rIdx = 0;
        g_rttViewerCb.aUp[idx].wIdx = 0;
        g_rttViewerCb.aUp[idx].flags = RTT_VIEWER_MODE_DEFAULT;
    }

    /* rtt viewer读缓冲区资源申请 */
    size = RTT_VIEWER_RB_MAX_NUM * RTT_VIEWER_RB_BUFFER_SIZE;
    rbuffer = (unsigned char *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, size);
    if ((rbuffer == NULL) || (memset_s(rbuffer, size, 0, size) != EOK)) {
        return;
    }

    /* rtt viewer读缓冲区初始化 */
    for (idx = 0; idx < RTT_VIEWER_RB_MAX_NUM; idx++) {
        g_rttViewerCb.aDown[idx].sName = "Terminal";
        g_rttViewerCb.aDown[idx].pBuffer = &rbuffer[idx * RTT_VIEWER_RB_BUFFER_SIZE];
        g_rttViewerCb.aDown[idx].size = RTT_VIEWER_RB_BUFFER_SIZE;
        g_rttViewerCb.aDown[idx].rIdx = 0;
        g_rttViewerCb.aDown[idx].wIdx = 0;
        g_rttViewerCb.aDown[idx].flags = RTT_VIEWER_MODE_DEFAULT;
    }

    /* 设置rtt viewer的id */
    strcpy((char *)&g_rttViewerCb.acID[0], "SEGGER");
    RTT_DMB();
    g_rttViewerCb.acID[6] = ' ';
    RTT_DMB();
    strcpy((char *)&g_rttViewerCb.acID[7], "RTT");
    RTT_DMB();

    return;
}

/*
 * 描述: rtt viewer写模式配置
 */
unsigned int RttViewerModeSet(unsigned int chn, unsigned int mode)
{
    if ((mode > RTT_VIEWER_MODE_MASK) || (chn >= RTT_VIEWER_WB_MAX_NUM)) {
      return OS_ERROR;
    }

    /* 设置模式 */
    g_rttViewerCb.aUp[chn].flags = mode;
}

/*
 * 描述: 参数校验
 */
static unsigned int RttViewerWriteParaCheck(unsigned int chn, const void *buffer, unsigned int numBytes)
{
    if (chn >= RTT_VIEWER_WB_MAX_NUM) {
        return OS_ERROR;
    }

    if (buffer == NULL) {
        return OS_ERROR;
    }

    if ((numBytes == 0) || (numBytes > RTT_VIEWER_WB_BUFFER_SIZE)) {
        return OS_ERROR;
    }

    return OS_OK;
}

/*
 * 描述: 获取可以写入环形缓冲区而不阻塞的字节数
 */
static unsigned int GetAvailWriteSpace(RTT_VIEWER_RING_BUFFER *ringBuffer) {
    unsigned int freeSize;

    if (ringBuffer->rIdx <= ringBuffer->wIdx) {
        freeSize = ringBuffer->size - 1 - ringBuffer->wIdx + ringBuffer->rIdx;
    } else {
        freeSize = ringBuffer->rIdx - ringBuffer->wIdx - 1;
    }

    return freeSize;
}

/*
 * 描述: 环缓冲区中存储指定数量的字符
 */
static void RingBufferPush(RTT_VIEWER_RING_BUFFER *ringBuffer, const char *data, unsigned int numBytes)
{
    unsigned int numBytesAtOnce;
    unsigned int wIdx;
    unsigned int rem;
    volatile char *dst;

    wIdx = ringBuffer->wIdx;
    rem = ringBuffer->size - wIdx;
    dst = ringBuffer->pBuffer + wIdx;
    if (rem > numBytes) {
        memcpy_s((void*)dst, numBytes, data, numBytes);
        RTT_DMB();
        ringBuffer->wIdx = wIdx + numBytes;
    } else {
        numBytesAtOnce = rem;
        memcpy_s((void*)dst, numBytesAtOnce, data, numBytesAtOnce);
        numBytesAtOnce = numBytes - rem;

        if (numBytesAtOnce > 0) {
            dst = ringBuffer->pBuffer;
            memcpy_s((void*)dst, numBytesAtOnce, data + rem, numBytesAtOnce);
            RTT_DMB();
        }
        ringBuffer->wIdx = numBytesAtOnce;
    }

    return;
}

/*
 * 描述: 环缓冲区中存储指定数量的字符，数据长度可以
 */
static unsigned int RingBufferBlockPush(RTT_VIEWER_RING_BUFFER *ringBuffer, const char *buffer, unsigned int numBytes) {
    unsigned int numBytesToWrite;
    unsigned int numBytesWritten;
    unsigned int rIdx;
    unsigned int wIdx;
    volatile char *dst;

    numBytesWritten = 0;
    wIdx = ringBuffer->wIdx;
    do {
        rIdx = ringBuffer->rIdx;
        if (rIdx > wIdx) {
            numBytesToWrite = rIdx - wIdx - 1u;
        } else {
            numBytesToWrite = ringBuffer->size - (wIdx - rIdx + 1u);
        }

        numBytesToWrite = MIN(numBytesToWrite, (ringBuffer->size - wIdx));
        numBytesToWrite = MIN(numBytesToWrite, numBytes);
        dst = ringBuffer->pBuffer + wIdx;
        memcpy_s((void*)dst, numBytesToWrite, buffer, numBytesToWrite);

        numBytesWritten += numBytesToWrite;
        buffer += numBytesToWrite;
        numBytes -= numBytesToWrite;
        wIdx += numBytesToWrite;
        if (wIdx == ringBuffer->size) {
            wIdx = 0;
        }
        RTT_DMB();
        ringBuffer->wIdx = wIdx;
    } while (numBytes);

    return numBytesWritten;
}

/*
 * 描述: rtt viewer写数据
 */
unsigned int RttViewerWrite(unsigned int chn, const void *buffer, unsigned int numBytes)
{
    unsigned int ret;
    unsigned int intSave;
    unsigned int freeSize;
    unsigned int writeBytes;
    RTT_VIEWER_RING_BUFFER *wRingBuffer;

    if (memcmp(g_rttViewerCb.acID, "SEGGER RTT", sizeof("SEGGER RTT")) != 0) {
        return OS_ERROR;
    }

    /* 校验入参 */
    ret = RttViewerWriteParaCheck(chn, buffer, numBytes);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = RttViewerLock();

    wRingBuffer = &g_rttViewerCb.aUp[chn];

    switch (wRingBuffer->flags) {
        case RTT_VIEWER_MODE_NO_BLOCK_SKIP:
            freeSize = GetAvailWriteSpace(wRingBuffer);
            if (freeSize >= numBytes) {
                RingBufferPush(wRingBuffer, buffer, numBytes);
            }
            break;

        case RTT_VIEWER_MODE_NO_BLOCK_TRIM:
            freeSize = GetAvailWriteSpace(wRingBuffer);
            RingBufferPush(wRingBuffer, buffer, MIN(freeSize, numBytes));
            break;

        case RTT_VIEWER_MODE_BLOCK_IF_FIFO_FULL:
            writeBytes = RingBufferBlockPush(wRingBuffer, buffer, numBytes);
            break;

        default:
            break;
    }

    RttViewerUnlock(intSave);

    return OS_OK;
}

/*
 * 描述: 参数校验
 */
unsigned int RttViewerReadParaCheck(unsigned int chn, const void *buffer, unsigned int numBytes)
{
    if (chn >= RTT_VIEWER_RB_MAX_NUM) {
        return OS_ERROR;
    }

    if (buffer == NULL) {
        return OS_ERROR;
    }

    if ((numBytes == 0) || (numBytes > RTT_VIEWER_RB_BUFFER_SIZE)) {
        return OS_ERROR;
    }

    return OS_OK;
}

/*
 * 描述: rtt viewer读数据
 */
unsigned int RttViewerRead(unsigned int chn, void *buffer, unsigned int numBytes)
{
    unsigned int ret;
    unsigned int intSave;
    unsigned int numBytesRem;
    unsigned int numBytesRead;
    unsigned int rIdx;
    unsigned int wIdx;
    volatile char *src;
    RTT_VIEWER_RING_BUFFER *rRingBuffer;

    if (memcmp(g_rttViewerCb.acID, "SEGGER RTT", sizeof("SEGGER RTT")) != 0) {
        return OS_ERROR;
    }

    /* 校验入参 */
    ret = RttViewerReadParaCheck(chn, buffer, numBytes);
    if (ret != OS_OK) {
        return 0;
    }

    intSave = RttViewerLock();

    rRingBuffer = &g_rttViewerCb.aDown[chn];
    rIdx = rRingBuffer->rIdx;
    wIdx = rRingBuffer->wIdx;
    numBytesRead = 0u;

    if (rIdx > wIdx) {
        numBytesRem = rRingBuffer->size - rIdx;
        numBytesRem = MIN(numBytesRem, numBytes);
        src = rRingBuffer->pBuffer + rIdx;
        memcpy_s(buffer, numBytesRem, (void*)src, numBytesRem);

        numBytesRead += numBytesRem;
        buffer += numBytesRem;
        numBytes -= numBytesRem;
        rIdx += numBytesRem;
        if (rIdx == rRingBuffer->size) {
            rIdx = 0u;
        }
    }

    numBytesRem = wIdx - rIdx;
    numBytesRem = MIN(numBytesRem, numBytes);
    if (numBytesRem > 0) {
        src = rRingBuffer->pBuffer + rIdx;
        memcpy_s(buffer, numBytesRem, (void*)src, numBytesRem);

        numBytesRead += numBytesRem;
        buffer += numBytesRem;
        numBytes -= numBytesRem;
        rIdx += numBytesRem;
    }

    if (numBytesRead) {
        rRingBuffer->rIdx = rIdx;
    }

    RttViewerUnlock(intSave);

    return numBytesRead;
}

/*
 * 描述: rtt viewer输出单个字符
 */
unsigned int RttViewerPutChar(unsigned int chn, char c)
{
    unsigned int ret;

    if (chn >= RTT_VIEWER_WB_MAX_NUM) {
        return OS_ERROR;
    }

    ret = RttViewerWrite(chn, &c, 1);

    return ret;
}

char g_rttViewerShowBuffer[RTT_VIEWER_MAX_SHOW_LEN];

/*
 * 描述: rtt viewer打印函数
 */
unsigned int RttViewerPrintf(unsigned int chn, const char *format, ...)
{
    unsigned int ret;
    int len;
    char *str = g_rttViewerShowBuffer;
    va_list vaList;

    if (chn >= RTT_VIEWER_WB_MAX_NUM) {
        return OS_ERROR;
    }

    va_start(vaList, format);
    len = vsnprintf_s(str, RTT_VIEWER_MAX_SHOW_LEN, RTT_VIEWER_MAX_SHOW_LEN, format, vaList);
    va_end(vaList);
    if (len == -1) {
        return len;
    }

    ret = RttViewerWrite(chn, str, len);

    return ret;
}
