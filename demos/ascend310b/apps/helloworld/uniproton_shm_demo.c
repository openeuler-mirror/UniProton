#include "shm_pub.h"
#include "prt_task.h"
#include "string.h"
#include "test.h"
#include "uniproton_shm_demo.h"

#define SHM_IPI_NUM 10
#define SHM_RD_ADDR 0x2BE00000
#define SHM_WR_ADDR 0x2BF00000

void ReadAndWrite(void)
{
    PRT_Printf("[uniproton] shm single test\n");
    shm_info_s *shm_rd = (shm_info_s *)SHM_RD_ADDR;
    shm_info_s *shm_wr= (shm_info_s *)SHM_WR_ADDR;
    int num = 9999;

    PRT_Printf("[uniproton] read from shm: %d\n", *(int *)shm_rd->data);
    shm_rd->op_type = SHM_OP_READ_END;

    *(int *)shm_wr->data = num--;
    PRT_Printf("[uniproton] write to shm: %d\n", num + 1);
    shm_wr->op_type = SHM_OP_READY_TO_READ;
}

char buf[0x2100] = {0};
void IpiHandle(uintptr_t para)
{
    shm_info_s *shm_rd = (shm_info_s *)SHM_RD_ADDR;
    PRT_Printf("[uniproton] IPI interupt. len = %u, ord = %d\n", shm_rd->used_size, shm_rd->resevered);
    memset(buf, 0, sizeof(buf));
    shm_read(shm_rd, buf, 0x2100);
    if (shm_rd->resevered == 0) {
        PRT_Printf("[uniproton]read from shm: %d\n", *(int *)buf);
    } else {
        PRT_Printf("[uniproton]read len: 0x%lx, st: %s, ed %s\n", shm_rd->used_size, buf, buf + shm_rd->used_size - 32);
    }
}

U32 TestShmStart(void)
{
    PRT_Printf("[uniproton] IPI init.\n");
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