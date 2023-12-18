#ifndef _RPC_CLIENT_INTERNAL_H
#define _RPC_CLIENT_INTERNAL_H

#include <openamp/rpmsg.h>
#include "prt_typedef.h"
#include "ioctl_rpc.h"

#define MAX_NODE_SIZE 2048
#define MAX_NODE_NUM 10
#define WORKERS 2
#define CMD_CLIENT_PRIORITY 9

extern U32 g_cmd_req_queue;

#endif /* _RPC_CLIENT_INTERNAL_H */