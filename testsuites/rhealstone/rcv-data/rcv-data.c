#include <prt_task.h>
#include <prt_sys.h>
#include "tmacros.h"
#include "timesys.h"
#include "prt_hwi.h"

#include <rpmsg_backend.h>

#define STR_SIZE 1024*1024

TskHandle taskId;
U32 status;


void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    char msg[] = "rtos rcv ok";
    char *buffer = NULL;
    int data_len = 0;
    int ret;

    buffer = (char*)malloc(STR_SIZE + 1);

    while(1) {
        memset(buffer, 0, STR_SIZE + 1);
        ret = rcv_data_from_nrtos(buffer, &data_len);
        if (ret != 0)
            break;
        printf("rcv buffer %s data_len %d \n", buffer + STR_SIZE - 10, data_len);
        send_data_to_nrtos(msg, sizeof(msg));
    }

    return;
}

void rcv_test()
{
    struct TskInitParam param = { 0 };

    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskId, &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    status = PRT_TaskResume(taskId);
    directive_failed(status, "PRT_TaskResume of TA01");

}