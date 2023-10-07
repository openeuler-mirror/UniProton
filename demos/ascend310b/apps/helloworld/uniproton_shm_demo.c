#include "shm_pub.h"
#include "prt_task.h"
#include "string.h"
#include "test.h"
#include "uniproton_shm_demo.h"

#define SHM_IPI_NUM 10

void ReadAndWrite(void)
{
    PRT_Printf("[uniproton] shm single test\n");
    shm_info_s *shm_read = (shm_info_s *)0x2BF00000;
    shm_info_s *shm_write = (shm_info_s *)0x2C000000;
    int num = 9999;

    PRT_Printf("[uniproton] read from shm: %d\n", *(int *)shm_read->data);
    shm_read->op_type = SHM_OP_READ_END;

    *(int *)shm_write->data = num--;
    PRT_Printf("[uniproton] write to shm: %d\n", num + 1);
    shm_write->op_type = SHM_OP_READY_TO_READ;
}

char buf[0x2100] = {0};
void IpiHandle(uintptr_t para)
{
    shm_info_s *shm = (shm_info_s *)0x2BF00000;
    PRT_Printf("[uniproton] IPI interupt. len = %u, ord = %d\n", shm->used_size, shm->resevered);
    memset(buf, 0, sizeof(buf));
    shm_read(shm, buf, 0x2100);
    if (shm->resevered == 0) {
        PRT_Printf("[uniproton]read from shm: %d\n", *(int *)buf);
    } else {
        PRT_Printf("[uniproton]read len: 0x%lx, st: %s, ed %s\n", shm->used_size, buf, buf + shm->used_size - 32);
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