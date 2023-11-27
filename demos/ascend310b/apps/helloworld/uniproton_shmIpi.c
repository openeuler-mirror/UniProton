#include "shm_pub.h"
#include "test.h"

int uniproton_shm_send_ipi(struct cpu_info info)
{
    unsigned int vmId = info.vmId;
    unsigned int ret = 0;
    asm volatile("mov x2, %[vmid] \n"
                 "mov x0, #1023 \n"
                 "mov x1, #10 \n"
                 "hvc #0 \n"
                 "mov %[result], x0\n"
                 : [result]"=r"(ret) : [vmid]"r"(vmId) : "x0");
    if (ret != 0) {
        PRT_Printf("send ipi to vm(%u) fail\n", vmId);
    }
    return 0;
}

struct shm_ipi_func uniproton_shmIpi = {
    .init = NULL,
    .send_ipi = uniproton_shm_send_ipi,
};