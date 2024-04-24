#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "ioctl_rpc.h"
#include "rpc_internal_model.h"
#include "rpc_client_internal.h"
#include "prt_queue.h"
#include "prt_task.h"
#include "prt_config.h"

#ifdef OS_SUPPORT_IGH_ETHERCAT

static TskHandle pids[WORKERS];

U32 g_cmd_req_queue = 0;

static int processed_num = 0;

int stub_ioctl(int fd, int request,  ...);
int stub_open();
void stub_close(int fd);
int is_special_request(unsigned int request);
void set_extra_mem_ptr(unsigned int request, void *arg);

static int rpmsg_handle_cmd_ioctl_req(struct rpmsg_endpoint *ept, cmd_base_req_t *req)
{
    int ret;
    cmd_ioctl_req_t *cmd_req = (cmd_ioctl_req_t *)req;
    size_t payload_size = sizeof(rpc_cmd_ioctl_req_t) + cmd_req->arg_size;
    rpc_cmd_ioctl_req_t *cmd_resp = malloc(payload_size);
    if (!cmd_resp) {
        return -1;
    }

    if (is_special_request(cmd_req->request)) {
        set_extra_mem_ptr(cmd_req->request, cmd_req->arg);
    }
    if (cmd_req->arg_size == 0) {
        void *param = *((void **)&(cmd_req->arg[0]));
        ret = stub_ioctl(cmd_req->fd, cmd_req->request, param);
    } else {
        ret = stub_ioctl(cmd_req->fd, cmd_req->request, cmd_req->arg);
    }
    cmd_resp->func_id = req->func_id;
    cmd_resp->cmd_pid = cmd_req->pid;
    cmd_resp->ioctl_respond.func_id = req->func_id;
    cmd_resp->ioctl_respond.ret = ret;
    cmd_resp->ioctl_respond.remote_errno = errno;
    cmd_resp->ioctl_respond.arg_size = cmd_req->arg_size;
    (void)memcpy_s(cmd_resp->ioctl_respond.arg, cmd_req->arg_size, cmd_req->arg, cmd_req->arg_size);
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    ret = rpmsg_send(ept, cmd_resp, payload_size);
    PRT_HwiRestore(intSave);
    free(cmd_resp);
    return ret;
}

static int rpmsg_handle_cmd_open_req(struct rpmsg_endpoint *ept, cmd_base_req_t *req)
{
    cmd_base_req_t *cmd_req = (cmd_base_req_t *)req;
    size_t payload_size = sizeof(rpc_cmd_base_req_t);
    rpc_cmd_base_req_t *cmd_resp = malloc(payload_size);
    if (!cmd_resp) {
        return -1;
    }

    int ret = stub_open();
    cmd_resp->func_id = req->func_id;
    cmd_resp->cmd_pid = cmd_req->pid;
    cmd_resp->base_respond.func_id = req->func_id;
    cmd_resp->base_respond.ret = ret;
    cmd_resp->base_respond.remote_errno = errno;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    ret = rpmsg_send(ept, cmd_resp, payload_size);
    PRT_HwiRestore(intSave);
    free(cmd_resp);
    return ret;
}

static int rpmsg_handle_cmd_close_req(struct rpmsg_endpoint *ept, cmd_base_req_t *req)
{
    cmd_close_req_t *cmd_req = (cmd_close_req_t *)req;
    size_t payload_size = sizeof(rpc_cmd_base_req_t);
    rpc_cmd_base_req_t *cmd_resp = malloc(payload_size);
    if (!cmd_resp) {
        return -1;
    }

    stub_close(cmd_req->fd);
    cmd_resp->func_id = req->func_id;
    cmd_resp->cmd_pid = cmd_req->pid;
    cmd_resp->base_respond.func_id = req->func_id;
    cmd_resp->base_respond.ret = 0;
    cmd_resp->base_respond.remote_errno = errno;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    int ret = rpmsg_send(ept, cmd_resp, payload_size);
    PRT_HwiRestore(intSave);
    free(cmd_resp);
    return ret;
}

static int process_cmd_request(struct rpmsg_endpoint *ept, cmd_base_req_t *req)
{
    int ret = 0;
    if (!req) {
        return -1;
    }

    switch (req->func_id) {
        case IGH_IOCTL_ID:
            return rpmsg_handle_cmd_ioctl_req(ept, req);
            break;
        case IGH_OPEN_ID:
            return rpmsg_handle_cmd_open_req(ept, req);
            break;
        case IGH_CLOSE_ID:
            return rpmsg_handle_cmd_close_req(ept, req);
            break;
        default:
            printf("[ERROR] invalid func_id:%lu\n", req->func_id);
            ret = -1;
            break;
    }
    return ret;
}

static void worker_thread(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    cmd_base_req_t *cmd_req = (cmd_base_req_t *)malloc(MAX_NODE_SIZE);
    U32 len = 2048;
    struct rpmsg_endpoint *ept = (struct rpmsg_endpoint *)param1;
    if (!cmd_req || !ept) {
        printf("[ERROR] woker thread null param!\n");
        return;
    }

    while (1) {
        if (PRT_QueueRead(g_cmd_req_queue, cmd_req, &len, OS_QUEUE_WAIT_FOREVER) != OS_OK) {
            printf("[ERROR] read data fail\n");
        } else {
            int ret = process_cmd_request(ept, cmd_req);
            processed_num++;
            if (ret < 0) {
                printf("[ERROR] process req fail\n");
            }
        }
        memset(cmd_req, 0, len);
        len = 2048;
    }
}

int workers_init(struct rpmsg_endpoint *ept)
{
    U32 ret = PRT_QueueCreate(MAX_NODE_NUM, MAX_NODE_SIZE, &g_cmd_req_queue);
    if (ret != OS_OK) {
        printf("[ERROR] create Q fail, ret:%u\n", ret);
        return ret;
    }

    for (int i = 0; i < WORKERS; i++) {
        struct TskInitParam param = {0};
        char tsk_name[16] = {0};
        sprintf(tsk_name, "IghCmdWorker%d", i);
        param.taskEntry = (TskEntryFunc)worker_thread;
        param.taskPrio = CMD_CLIENT_PRIORITY;
        param.args[0] = (uintptr_t)ept;
        param.stackSize = OS_TSK_DEFAULT_STACK_SIZE;
        param.name = tsk_name;
        param.stackAddr = 0;
        
        U32 ret = PRT_TaskCreate(pids+i, &param);
        if (ret != OS_OK) {
            printf("[ERROR] fail to create, ret:%u\n", ret);
            return ret;
        }
        ret = PRT_TaskResume(pids[i]);
        if (ret != OS_OK) {
            printf("[ERROR] fail to resume, ret:%u\n", ret);
            return ret;
        }
    }
    printf("igh cmd workers created\n");
    return 0;
}
#endif