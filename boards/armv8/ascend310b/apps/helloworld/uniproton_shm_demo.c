#include "shm_pub.h"
#include "prt_task.h"
#include "string.h"
#include "test.h"
#include "uniproton_shm_demo.h"

#define SHM_IPI_NUM 10
#define SHM_RD_ADDR 0x2A800000
#define SHM_WR_ADDR 0x2AC00000
#define IPI_TARGET 0

void ShmSendEntry()
{
    shm_info_s *shm_wr = (shm_info_s *)SHM_WR_ADDR;
    unsigned long long t;

    /* 整数发送测试 */
    int tmp = 8080808;
    for (int i = 0; i < 10; i++) {
        shm_write(&tmp, sizeof(tmp), 0, shm_wr, IPI_TARGET);
        PRT_Printf("[uniproton] write to shm int: %d\n", tmp--);
        PRT_TaskDelay(OS_TICK_PER_SECOND);
    }

    /* 字符串发送测试 */
    char str[] = "Hello openEuler";
    shm_write(str, sizeof(str), 1, shm_wr, IPI_TARGET);
    PRT_Printf("[uniproton] write string: %s\n", str);
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    /* 通信响应测试 */
    while (1) {
        shm_wr->used_size = sizeof(unsigned long long);
        shm_wr->resevered = 2;
        shm_wr->op_type = SHM_OP_READY_TO_READ;
        asm volatile("mrs %0, CNTPCT_EL0\n" : "=r" (t));
        *(U64 *)shm_wr->data = t;
        shm_send_ipi(IPI_TARGET);
        PRT_TaskDelay(OS_TICK_PER_SECOND * 3);
    }
}

U32 TestShmSend(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle tskHandle;

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x6000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)ShmSendEntry;
    param.taskPrio = 25;
    param.name = "ShmWriteTask";
    param.stackSize = 0x6000;

    ret = PRT_TaskCreate(&tskHandle, &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(tskHandle);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

char g_buf[0x2100] = {0};
void IpiHandle(uintptr_t para)
{
    shm_info_s *shm_rd = (shm_info_s *)SHM_RD_ADDR;
    unsigned long long t;
    static int wr_start = 0;

    if (wr_start == 0) {
        TestShmSend();
        wr_start = 1;
    }

    switch (shm_rd->resevered) {
        case 0:
            /* 整数读取测试 */
            shm_read(shm_rd, g_buf, sizeof(g_buf));
            PRT_Printf("[uniproton]read from shm: %d\n", *(int *)g_buf);
            break;
        case 1:
            /* 字符串读取测试 */
            shm_read(shm_rd, g_buf, sizeof(g_buf));
            PRT_Printf("[uniproton]read len: 0x%lx, st: %s\n", shm_rd->used_size, g_buf);
            break;
        case 2:
            /* 通信响应测试 */
            asm volatile("mrs %0, CNTPCT_EL0\n" : "=r" (t));
            shm_rd->op_type = SHM_OP_READ_END;
            PRT_Printf("[uniproton]read period %llu us\n", (t - *(U64 *)shm_rd->data) / 48);
            break;
        default:
            break;
    }
}

U32 TestShmStart(void)
{
    PRT_Printf("[uniproton] VM communication init.\n");
    shm_ipi_init();

    U32 ret = PRT_HwiSetAttr(SHM_IPI_NUM, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiCreate(SHM_IPI_NUM, (HwiProcFunc)IpiHandle, 0);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiEnable(SHM_IPI_NUM);
    if (ret != OS_OK) {
        return ret;
    }
    return OS_OK;
}