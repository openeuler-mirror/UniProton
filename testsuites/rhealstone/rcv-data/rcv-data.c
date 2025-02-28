#include <prt_task.h>
#include <prt_sys.h>
#include "tmacros.h"
#include "timesys.h"
#include "prt_hwi.h"

#include <rpmsg_backend.h>

#define STR_SIZE 1024*1024

TskHandle taskId;
U32 status;

void create_random_char(char *buffer) 
{
    for (int i = 0; i < STR_SIZE; i++) {
        buffer[i] = 'B' + rand() % 26;
    }
}

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    char msg[] = "rtos rcv ok";
    char *rcv_buffer = NULL , *result_buffer = NULL;
    int data_len = 0;
    int ret = 0, i = 0;

    srand(time(NULL));
    rcv_buffer = (char*)malloc(STR_SIZE + 1);
    if (rcv_buffer == NULL) {
        printf("rcv_buffer malloc failed\n");
    }
    result_buffer = (char*)malloc(STR_SIZE + 1);
    if (result_buffer == NULL) {
        printf("result_buffer malloc failed\n");
    }

    while(1) {
        memset(rcv_buffer, 0, STR_SIZE + 1);
        memset(result_buffer, 0, STR_SIZE + 1);
        ret = rcv_data_from_nrtos(rcv_buffer, &data_len);
        if (ret != 0)
            break;
        printf("receive buffer last ten charactors %s, total receive buffer len is  %d \n", rcv_buffer + STR_SIZE - 10, data_len);
        create_random_char(result_buffer);
        printf("send buffer last ten charactors  %s, total send buffer len is %d \n", result_buffer + STR_SIZE - 10, STR_SIZE);
        send_data_to_nrtos(result_buffer, STR_SIZE);
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
