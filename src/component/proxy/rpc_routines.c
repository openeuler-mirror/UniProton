/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-30
 * Description: 代理接口客户端实现
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>

#include <openamp/rpmsg.h>
#include <openamp/rpmsg_rpc_client_server.h>

#include "securec.h"
#include "rpc_internal_common.h"
#include "rpc_internal_model.h"
#include "rpc_err.h"

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT    97
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ    89
#endif

#define DEFINE_CB_VARS(name)                                    \
    rpc_##name##_resp_t *resp = (rpc_##name##_resp_t *)from;    \
    rpc_##name##_outp_t *outp = (rpc_##name##_outp_t *)to;

#define DEFINE_COMMON_RPC_VAR(name)             \
    rpc_##name##_req_t req;                     \
    rpc_##name##_outp_t outp;                   \
    int slot_idx = 0, ret = 0;                  \
    unsigned int payload_size = sizeof(req);    \
    outp.super.eptr = &errno;

#define RECORD_AT(i) (g_records_ctl->records[i])

#define CHECK_COND(cond, ret)                   \
    if (cond) {                                 \
        return ret;                             \
    }

#define CHECK_RET(ret)    CHECK_COND(ret < 0, ret)

#define CHECK(cond)    CHECK_COND(cond, -EINVAL)

#define CHECK_RET_NULL(cond) CHECK_COND(cond, NULL)

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define CHECK_AND_SET_ERRNO(cond, ret_value, err_value)        \
    if (cond) {                                                \
        errno = err_value;                                     \
        return ret_value;                                      \
    }

#define CHECK_ARG(cond, err_value)                 \
        CHECK_AND_SET_ERRNO(cond, -1, err_value)

#define CHECK_INIT()    CHECK_COND(g_ept == NULL, -RPC_ENEED_INIT)

#define MAX_RECORDS 128

#define WEAK_ALIAS(old, new) \
    extern __typeof(old) new __attribute__((__weak__, __alias__(#old)))

#define dprintf(format, ...)  

#define CONVERT(name) resp2outp_##name
#define DEF_CONVERT(name)  \
    static void CONVERT(name) (void *from, void *to)

typedef void (*resp2outp_fn) (void *from, void *to);
typedef struct rpc_record {
    uint32_t trace_id;
    bool in_use;
    void *outp;
    resp2outp_fn cb;
} rpc_record_t;

typedef struct rpc_records_ctl {
    rpc_record_t *const records;
    int *const status;
    const uint32_t len;
    uint32_t trace_counter; 
    pthread_mutex_t lock; 
} rpc_records_ctl_t;

static struct rpmsg_endpoint *g_ept;

static rpc_record_t g_records[MAX_RECORDS];
static int g_record_status[MAX_RECORDS];
static rpc_records_ctl_t g_default_ctl = {
    .records = g_records,
    .status = g_record_status,
    .len = MAX_RECORDS,
    .trace_counter = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER
};
static rpc_records_ctl_t *g_records_ctl = &g_default_ctl;

static int alloc_slot()
{
    uint32_t counter;
    int slot_idx = -1;
    int len = g_records_ctl->len;
    rpc_record_t *record = NULL;

    pthread_mutex_lock(&g_records_ctl->lock);
    counter = g_records_ctl->trace_counter;
    for (int i = 0, idx; i < len; i++, counter++) {
        idx = counter & (len - 1);
        record = &RECORD_AT(idx);
        if (record != NULL && !record->in_use) {
            slot_idx = idx;
            record->in_use = true;
            record->trace_id = counter;
            record->outp = NULL;
            record->cb = NULL;
            g_records_ctl->trace_counter = counter + 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_records_ctl->lock);
    return slot_idx;
}

static int new_slot(void *outp)
{
    int idx = alloc_slot();
 
    if (idx < 0) {
        return -RPC_ENO_SLOT;
    }
    RECORD_AT(idx).outp = outp;

    return idx;
}

static inline int tid2si(int trace_id)
{
    return trace_id & (g_records_ctl->len - 1);
}

static int free_slot(int idx) 
{
    if (idx < 0 || idx >= g_records_ctl->len) {
        return -1;
    }

    pthread_mutex_lock(&g_records_ctl->lock);
    RECORD_AT(idx).in_use = false;
    pthread_mutex_unlock(&g_records_ctl->lock);
    return 0;
}

static void set_status(int idx, int status)
{
    g_record_status[idx] = status;
}

static bool not_ready(int idx)
{
    return g_record_status[idx] != STATUS_READY;
}

static void common_cb(int status, void *data, size_t len)
{
    int idx;
    resp2outp_fn cb;
    rpc_resp_base_t *base;
    rpc_outp_base_t *obase;
    UNUSED(status);
    UNUSED(len);

    base = (rpc_resp_base_t *)data;
    int trace_id = base->trace_id;
    idx = tid2si(trace_id);
    dprintf("tid:%d", trace_id);
    obase = RECORD_AT(idx).outp;
    cb = RECORD_AT(idx).cb;
    if (obase != NULL && obase->eptr != NULL) {
        *obase->eptr = base->errnum;
    }
    if (cb != NULL) {
        cb(data, obase);
    }
    /* to clear the flag set in the caller function */
    set_status(idx, STATUS_READY);
}

DEF_CONVERT(common)
{
    DEFINE_CB_VARS(common)

    outp->ret = resp->ret;
}

DEF_CONVERT(csret)
{
    DEFINE_CB_VARS(csret)

    outp->ret = resp->ret;
}

DEF_CONVERT(read)
{
    DEFINE_CB_VARS(read)
    size_t buflen = MIN(sizeof(resp->buf), resp->ret);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->ret = buflen;
}

DEF_CONVERT(ioctl)
{
    DEFINE_CB_VARS(ioctl)
    size_t buflen = MIN(sizeof(resp->buf), resp->len);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->ret = resp->ret;
}

void rpmsg_set_default_ept(struct rpmsg_endpoint *ept)
{
    if (ept == NULL)
        return;
    g_ept = ept;
}

int rpmsg_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv)
{
    struct rpmsg_rpc_answer *msg;
    UNUSED(priv);
    UNUSED(src);

    CHECK_COND(data == NULL, RPMSG_ERR_ADDR)
    CHECK_COND(ept == NULL, RPMSG_ERR_INIT)
    msg = (struct rpmsg_rpc_answer *)data;
    dprintf("==(%x,%d)", src, msg->id);

    if (msg->id < MIN_ID || msg->id > MAX_ID) {
        dprintf("invalid msg id(%d)\n", msg->id);
        return RPMSG_ERR_PARAM;
    }
    /* Invoke the callback function of the rpc */
    common_cb(msg->status, msg->params, len);
    return RPMSG_SUCCESS;
}

static int wait4resp(int slot_idx, void *data, size_t size)
{
    int ret = 0;

    /* flag set to wait for response from endpoint callback */
    set_status(slot_idx, STATUS_WAITING);
    ret = rpmsg_send(g_ept, data, size);
    if (ret < 0) {
        set_status(slot_idx, STATUS_READY);
        free_slot(slot_idx);
        dprintf("sendfail:%d\n", ret);
        return ret;
    }
    /* waiting to get response from endpoint callback */
    while (not_ready(slot_idx)) {
        sched_yield();
    }
    return 0;
}

static int __open(const char *filename, int flags, unsigned mode)
{
    DEFINE_COMMON_RPC_VAR(open)
    int len = 0;

    CHECK_INIT()
    CHECK_ARG(filename == NULL, EFAULT)
    len = strlen(filename) + 1;
    CHECK_ARG(len > sizeof(req.buf), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = OPEN_ID;
    req.flags = flags;
    req.mode = mode;
    memcpy_s(req.buf, len, filename, len);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    payload_size = payload_size - sizeof(req.buf) + len;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyOpen(const char *filename, int flags, ...)
{
    mode_t mode = 0;

    va_list ap;
    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);

    return __open(filename, flags, mode);
}

ssize_t PRT_ProxyRead(int fd, void *buf, size_t count)
{
    DEFINE_COMMON_RPC_VAR(read)
    size_t cnt = MIN(count, MAX_STRING_LEN);
    ssize_t sret = 0;

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    /* store buffer address and size  for the callback */
    outp.buf = buf;
    outp.ret = 0;

    /* Construct rpc payload */
    req.func_id = READ_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    req.count = cnt;

    RECORD_AT(slot_idx).cb = CONVERT(read);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

ssize_t PRT_ProxyWrite(int fd, const void *buf, size_t count)
{
    DEFINE_COMMON_RPC_VAR(write)
    ssize_t sret = 0;
    count = MIN(count, sizeof(req.buf));

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = WRITE_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    memcpy_s(req.buf, count, buf, count);
    req.count = count;
    payload_size = payload_size - sizeof(req.buf) + count;

    RECORD_AT(slot_idx).cb = CONVERT(csret);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

int PRT_ProxyClose(int fd)
{
    DEFINE_COMMON_RPC_VAR(close)

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = CLOSE_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyIoctl(int fd, unsigned long request, void *arg, size_t len)
{
    DEFINE_COMMON_RPC_VAR(ioctl)

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = IOCTL_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    req.request = request;
    req.len = MIN(len, sizeof(req.buf));
    payload_size -= sizeof(req.buf);
    if (arg != NULL && req.len > 0) {
        memcpy_s(req.buf, req.len, arg, req.len);
        payload_size += req.len;
    }
    RECORD_AT(slot_idx).cb = CONVERT(ioctl);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

static int __fcntl(int fd, int cmd, unsigned long arg)
{
    DEFINE_COMMON_RPC_VAR(fcntl)

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = FCNTL_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    req.cmd = cmd;
    req.arg = arg;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}


int PRT_ProxyFcntl(int fd, int cmd, ...)
{
    unsigned long arg;
    va_list ap;
    va_start(ap, cmd);
    arg = va_arg(ap, unsigned long);
    va_end(ap);
    return __fcntl(fd, cmd, arg);
}

 off_t PRT_ProxyLseek(int fd, off_t offset, int whence)
 {
    DEFINE_COMMON_RPC_VAR(lseek)
    off_t oret = 0;

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = LSEEK_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    req.offset = offset;
    req.whence = whence;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    oret = outp.ret;
    free_slot(slot_idx);
    return oret;
 }

int PRT_ProxyUnlink(const char *path)
{
    DEFINE_COMMON_RPC_VAR(unlink)
    int len = 0;

    CHECK_INIT()
    CHECK_ARG(path == NULL, EFAULT)
    len = strlen(path) + 1;
    CHECK_ARG(len > sizeof(req.buf), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = UNLINK_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;

    memcpy_s(req.buf, len, path, len);
    payload_size = payload_size - sizeof(req.buf) + len;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

WEAK_ALIAS(PRT_ProxyOpen, open);
WEAK_ALIAS(PRT_ProxyRead, read);
WEAK_ALIAS(PRT_ProxyWrite, write);
WEAK_ALIAS(PRT_ProxyClose, close);
WEAK_ALIAS(PRT_ProxyLseek, lseek);
WEAK_ALIAS(PRT_ProxyFcntl, fcntl);
WEAK_ALIAS(PRT_ProxyUnlink, unlink);
