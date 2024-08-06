#include "emmc.h"
#include "shm_pub.h"
#include "cpu_config.h"

static shm_info_s *g_shm_wr;
/* 这里只支持虚拟化场景 */
void EMMC_WriteFilePrepare(void)
{
    g_shm_wr = (shm_info_s *)SHM_WR_ADDR;
    g_shm_wr->used_size = 0;
}

U32 EMMC_WriteFile(const char *data, U32 size)
{
    U32 i;
    for (i = 0; i < size; i++) {
        *((char*)g_shm_wr->data + g_shm_wr->used_size + i) = data[i];
    }
    g_shm_wr->used_size += size;
    return 0;
}

U32 EMMC_WriteFileComplete(U32 size)
{
    if (g_shm_wr->used_size != size) {
        return 1;
    }
    asm volatile("mov x0, #1025 \n"
                 "hvc #0 \n"
                 : : : "memory");
    return 0;
}
