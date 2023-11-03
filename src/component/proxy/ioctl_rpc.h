#ifndef _IOCTL_RPC_H
#define _IOCTL_RPC_H

#include <sys/types.h>

#define IGH_IOCTL_ID        301UL
#define IGH_OPEN_ID         302UL
#define IGH_CLOSE_ID        303UL

#define MQ_NAME_FORMATE "/mq_slave_%d"
#define MQ_MASTER_NAME "/mq_0"

typedef struct {
    unsigned long func_id;
    pid_t pid;
} cmd_base_req_t; // open can just use base req

typedef struct {
    unsigned long func_id;
    int ret;
    int remote_errno;
} cmd_base_resp_t; // open and close can just use base resp

typedef struct {
    unsigned long func_id;
    pid_t pid;
    int fd;
} cmd_close_req_t;

typedef struct {
    unsigned long func_id;
    pid_t pid;
    int fd;
    unsigned int request;
    int arg_size;
    char arg[]; // flexible arg
} cmd_ioctl_req_t;

typedef struct {
    unsigned long func_id;
    int ret;
    int remote_errno;
    int arg_size;
    char resv[4]; // 8字节对齐
    char arg[]; // flexible arg
} cmd_ioctl_resp_t;

#define is_valid_cmd_func_id(func_id) ((func_id) >= IGH_IOCTL_ID && (func_id) <= IGH_CLOSE_ID)

#endif /* _IOCTL_RPC_H */