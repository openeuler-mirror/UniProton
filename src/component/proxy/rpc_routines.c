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

DEF_CONVERT(gethostbyaddr)
{
    DEFINE_CB_VARS(gethostbyaddr)
    size_t buflen = MIN(sizeof(resp->buf), resp->len);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->len = buflen;
    // todo: set h_errno
}

DEF_CONVERT(getaddrinfo)
{
    DEFINE_CB_VARS(getaddrinfo)
    size_t buflen = MIN(sizeof(resp->buf), resp->buflen);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->cnt = resp->cnt;
    outp->ret = resp->ret;
}

DEF_CONVERT(getpeername)
{
    DEFINE_CB_VARS(getpeername)
    size_t buflen = MIN(sizeof(resp->addr_buf), resp->addrlen);

    if (outp->addr != NULL && buflen > 0) {
        memcpy_s(outp->addr, buflen, resp->addr_buf, buflen);
    }
    if (outp->addrlen != NULL) {
        *outp->addrlen = buflen;
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(accept)
{
    DEFINE_CB_VARS(accept)
    size_t buflen = MIN(sizeof(resp->buf), resp->addrlen);

    if (outp->addrlen != NULL) {
        *outp->addrlen = buflen;
    }
    if (outp->addr != NULL && buflen > 0) {
        memcpy_s(outp->addr, buflen, resp->buf, buflen);
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(recv)
{
    DEFINE_CB_VARS(recv)
    size_t buflen = MIN(sizeof(resp->buf), resp->ret);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->ret = buflen;
}

DEF_CONVERT(recvfrom)
{
    DEFINE_CB_VARS(recvfrom)

    if (outp->buf != NULL && resp->ret > 0) {
        memcpy_s(outp->buf, resp->ret, resp->buf, resp->ret);
    }
    if (resp->addrlen > 0 && outp->src_addr != NULL) {
        memcpy_s(outp->src_addr, resp->addrlen, resp->addr, resp->addrlen);
    }
    if (outp->addrlen != NULL) {
        *outp->addrlen = resp->addrlen;
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(poll)
{
    DEFINE_CB_VARS(poll)
    int len = sizeof(struct pollfd) * (outp->fdsNum);

    if (outp->fds != NULL) {
        memcpy_s(outp->fds, len, resp->fds, len);
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(select)
{
    DEFINE_CB_VARS(select)

    if (outp->readfds != NULL) {
        memcpy_s(outp->readfds, sizeof(fd_set), &(resp->readfds), sizeof(fd_set));
    }

    if (outp->writefds != NULL) {
        memcpy_s(outp->writefds, sizeof(fd_set), &(resp->writefds), sizeof(fd_set));
    }
    
    if (outp->exceptfds != NULL) {
        memcpy_s(outp->exceptfds, sizeof(fd_set), &(resp->exceptfds), sizeof(fd_set));
    }

    if (outp->timeout != NULL) {
        memcpy_s(outp->timeout, sizeof(struct timeval), &(resp->timeout), 
        sizeof(struct timeval));
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(gethostname)
{
    DEFINE_CB_VARS(gethostname)
    size_t buflen = MIN(sizeof(resp->name), resp->len);

    if (outp->name != NULL) {
        memcpy_s(outp->name, buflen, resp->name, buflen);
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(getsockopt)
{
    DEFINE_CB_VARS(getsockopt)
    outp->ret = resp->ret;

    if (outp->optval == NULL) {
        return;
    }
    if (resp->optlen > MAX_STRING_LEN) {
        outp->ret = -1;
        return;
    }
    memcpy_s(outp->optval, resp->optlen, resp->optval, resp->optlen);
    if (outp->optlen != NULL) {
        *outp->optlen = resp->optlen;
    }
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

void PRT_ProxyFreeAddrInfo(struct addrinfo *ai)
{
    while (ai != NULL) {
        struct addrinfo *p = ai;
        ai = ai->ai_next;  
        free(p->ai_canonname);
        free(p);
    }
}

int PRT_ProxyGetAddrInfo(const char *node, const char *service,
                    const struct addrinfo *hints,
                    struct addrinfo **res)
{
    DEFINE_COMMON_RPC_VAR(getaddrinfo)
    int nlen = 0, slen = 0, hlen = sizeof(req.buf);

    CHECK_INIT()
    CHECK_ARG(res == NULL, EFAULT)
    req.hints_cnt = req.hints = req.node = req.service = 0;
    if (node != NULL) {
        nlen = strlen(node) + 1;
        req.service += nlen;
    }
    if (service != NULL) {
        slen = strlen(service) + 1;
    }
    req.buflen = nlen + slen;
    CHECK_ARG(req.buflen > sizeof(req.buf), EINVAL)
    if (hints != NULL) {
        req.hints_cnt = OsProxyEncodeAddrList(hints, req.buf, &hlen);
        req.service += hlen;
        req.node += hlen;
        req.buflen += hlen;
        CHECK_ARG(req.hints_cnt < 0, EINVAL)
        CHECK_ARG(req.buflen > sizeof(req.buf), EINVAL)
    }
    if (node != NULL) {
        memcpy_s(&req.buf[req.node], nlen, node, nlen);
    }
    if (service != NULL) {
        memcpy_s(&req.buf[req.service], slen, service, slen);
    }
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = GETADDRINFO_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    outp.buf = req.buf;
    payload_size = payload_size - sizeof(req.buf) + req.buflen;

    RECORD_AT(slot_idx).cb = CONVERT(getaddrinfo);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    if (outp.ret) {
        ret = outp.ret;
    } else if (outp.cnt > 0) {
        ret = OsProxyDecodeAddrList(req.buf, outp.cnt, sizeof(req.buf), res);
    }

    free_slot(slot_idx);
    return ret;
}

struct hostent *PRT_ProxyGetHostByAddr(const void *addr, socklen_t len, int type)
{
    DEFINE_COMMON_RPC_VAR(gethostbyaddr)
    struct hostent *ht = NULL;

    CHECK_RET_NULL(g_ept == NULL)
    CHECK_AND_SET_ERRNO(len == 0, NULL, EAFNOSUPPORT)
    CHECK_AND_SET_ERRNO(addr == NULL, NULL, EFAULT)
    CHECK_AND_SET_ERRNO(len > sizeof(req.buf), NULL, EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET_NULL(slot_idx < 0)

    req.func_id = GETHOSTBYADDR_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    memcpy_s(req.buf, len, addr, len);
    req.len = len;
    req.type = type;

    outp.buf = req.buf;
    outp.len = sizeof(req.buf);

    payload_size = payload_size - sizeof(req.buf) + len;

    RECORD_AT(slot_idx).cb = CONVERT(gethostbyaddr);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET_NULL(ret < 0)

    if (outp.len <= 0 || outp.len > sizeof(req.buf)) {
        ht = NULL;
    } else {
        ret = OsProxyDecodeHostEnt(&ht, req.buf, outp.len);
        if (ret) {
            ht = NULL;
        }
    }

    free_slot(slot_idx);
    return ht;
}

struct hostent *PRT_ProxyGetHostByName(const char *name)
{
    DEFINE_COMMON_RPC_VAR(gethostbyname)
    struct hostent *ht = NULL;
    int len = 0;

    CHECK_RET_NULL(g_ept == NULL)
    CHECK_AND_SET_ERRNO(name == NULL, NULL, EFAULT)
    len = strlen(name) + 1;
    slot_idx = new_slot(&outp);
    CHECK_RET_NULL(slot_idx < 0)

    req.func_id = GETHOSTBYNAME_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    memcpy_s(req.buf, len, name, len);
    outp.buf = req.buf;
    outp.len = sizeof(req.buf);
    payload_size = payload_size - sizeof(req.buf) + len;

    RECORD_AT(slot_idx).cb = CONVERT(gethostbyaddr);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET_NULL(ret < 0)

    if (outp.len <= 0 || outp.len > sizeof(req.buf)) {
        ht = NULL;
    } else {
        ret = OsProxyDecodeHostEnt(&ht, req.buf, outp.len);
        if (ret) {
            ht = NULL;
        }
    }

    free_slot(slot_idx);
    return ht;
}

int PRT_ProxyPoll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    DEFINE_COMMON_RPC_VAR(poll)

    CHECK_INIT()
    CHECK_ARG(nfds > MAX_POLL_FDS, EINVAL);
    CHECK_ARG(fds == NULL, EFAULT);
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    /* store buffer address and size  for the callback */
    outp.fds = fds;
    outp.fdsNum = (int)nfds;

    /* Construct rpc payload */
    req.func_id = POLL_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    memcpy_s(req.fds, sizeof(req.fds), fds,
            sizeof(struct pollfd) * nfds);
    req.nfds = nfds;
    req.timeout = timeout;

    RECORD_AT(slot_idx).cb = CONVERT(poll);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyGetPeerName(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    DEFINE_COMMON_RPC_VAR(getpeername)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addr == NULL || addrlen == NULL, EFAULT)
    CHECK_ARG(*addrlen > sizeof(req.addr_buf), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = GETPEERNAME_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.addrlen = *addrlen;
    if (req.addrlen > 0) {
        memcpy_s(req.addr_buf, req.addrlen, addr, req.addrlen);
    }
    outp.addr = addr;
    outp.addrlen = addrlen;
    payload_size = payload_size - sizeof(req.addr_buf) + req.addrlen;

    RECORD_AT(slot_idx).cb = CONVERT(getpeername);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyGetHostName(char *name, size_t len)
{
    DEFINE_COMMON_RPC_VAR(gethostname)

    CHECK_INIT()
    CHECK_ARG(len == 0, ENAMETOOLONG)
    CHECK_ARG(name == NULL, EFAULT)
    len = MIN(len, MAX_STRING_LEN);
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    /* store buffer address and size  for the callback */
    outp.name = name;
    outp.len = len;

    /* Construct rpc payload */
    req.func_id = GETHOSTNAME_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.len = len;
    
    RECORD_AT(slot_idx).cb = CONVERT(gethostname);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyGetSockName(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    DEFINE_COMMON_RPC_VAR(getsockname)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addr == NULL || addrlen == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = GETSOCKNAME_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.addrlen = MIN(sizeof(req.addr_buf), *addrlen);
    if (req.addrlen > 0) {
        memcpy_s(req.addr_buf, req.addrlen, addr, req.addrlen);
    }
    outp.addr = addr;
    outp.addrlen = addrlen;
    payload_size = payload_size - sizeof(req.addr_buf) + req.addrlen;

    RECORD_AT(slot_idx).cb = CONVERT(getpeername);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyGetSockOpt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    DEFINE_COMMON_RPC_VAR(getsockopt)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(optval == NULL || optlen == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    /* store buffer address and size  for the callback */
    outp.optval = optval;
    outp.optlen = optlen;

    /* Construct rpc payload */
    req.func_id = GETSOCKOPT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.level = level;
    req.optname = optname;
    req.optlen = *optlen;

    RECORD_AT(slot_idx).cb = CONVERT(getsockopt);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxySelect(int n, fd_set *restrict rfds, fd_set *restrict wfds, 
                 fd_set *restrict efds, struct timeval *restrict tv)
{
    DEFINE_COMMON_RPC_VAR(select)

    CHECK_INIT()
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    /* store buffer address and size  for the callback */
    outp.readfds = rfds;
    outp.writefds = wfds;
    outp.exceptfds = efds;
    outp.timeout = tv;

    /* Construct rpc payload */
    req.func_id = SELECT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.nfds = n;
    req.is_readfds_not_null = 0;
    req.is_writefds_not_null = 0;
    req.is_exceptfds_not_null = 0;
    req.is_timeout_not_null = 0;
    if (rfds != NULL) {
        memcpy_s(&req.readfds, sizeof(req.readfds), rfds, sizeof(fd_set));
        req.is_readfds_not_null = 1;
    }

    if (wfds != NULL) {
        memcpy_s(&req.writefds, sizeof(req.writefds), wfds, sizeof(fd_set));
        req.is_writefds_not_null = 1;
    }

    if (efds != NULL) {
        memcpy_s(&req.exceptfds, sizeof(req.exceptfds), efds, sizeof(fd_set));
        req.is_exceptfds_not_null = 1;
    }

    if (tv != NULL) {
        memcpy_s(&req.timeout, sizeof(req.timeout), tv, 
                sizeof(struct timeval));
        req.is_timeout_not_null = 1;
    }

    RECORD_AT(slot_idx).cb = CONVERT(select);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    DEFINE_COMMON_RPC_VAR(accept)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addr != NULL && addrlen == NULL , EFAULT)
    CHECK_ARG(addrlen != NULL && *addrlen > sizeof(req.addr_buf), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = ACCEPT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    if (addr != NULL && addrlen != NULL) {
        req.addrlen = *addrlen;
        memcpy_s(req.addr_buf, req.addrlen, addr, req.addrlen);
    } else {
        req.addrlen = 0;
        payload_size -= sizeof(req.addr_buf);
    }

    outp.addr = addr;
    outp.addrlen = addrlen;

    RECORD_AT(slot_idx).cb = CONVERT(accept);
    ret = wait4resp(slot_idx, &req, payload_size); // todo: optimization
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    DEFINE_COMMON_RPC_VAR(bind)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addrlen == 0 || addrlen > sizeof(req.addr_buf), EINVAL)
    CHECK_ARG(addr == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = BIND_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.addrlen = addrlen;
    memcpy_s(req.addr_buf, addrlen, addr, addrlen);

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    DEFINE_COMMON_RPC_VAR(connect)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addrlen == 0 || addrlen > sizeof(req.addr_buf), EINVAL)
    CHECK_ARG(addr == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = CONNECT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.addrlen = addrlen;
    memcpy_s(req.addr_buf, addrlen, addr, addrlen);

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyListen(int sockfd, int backlog)
{
    DEFINE_COMMON_RPC_VAR(listen)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = LISTEN_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.backlog = backlog;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

ssize_t PRT_ProxyRecv(int sockfd, void *buf, size_t len, int flags)
{
    DEFINE_COMMON_RPC_VAR(recv)
    ssize_t sret = 0;

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    outp.buf = buf;

    req.func_id = RECV_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.len = MIN(MAX_SBUF_LEN, len);
    req.flags = flags;

    RECORD_AT(slot_idx).cb = CONVERT(recv);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

static ssize_t __PRT_ProxyRecvFrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen)
{
    DEFINE_COMMON_RPC_VAR(recvfrom)
    ssize_t sret = 0;

    CHECK_ARG(*addrlen > sizeof(req.buf), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    outp.buf = buf;
    outp.addrlen = addrlen;
    outp.src_addr = src_addr;

    req.func_id = RECVFROM_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.addrlen = *addrlen;
    req.len = MIN(sizeof(req.buf) - req.addrlen, len);
    req.flags = flags;
    if (req.addrlen > 0) {
        memcpy_s(req.buf, req.addrlen, src_addr, req.addrlen);
    }
    payload_size = payload_size - sizeof(req.buf) + req.addrlen;
    RECORD_AT(slot_idx).cb = CONVERT(recvfrom);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

ssize_t PRT_ProxyRecvFrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen)
{
    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    if (src_addr == NULL) {
        return PRT_ProxyRecv(sockfd, buf, len, flags);
    }
    CHECK_ARG(addrlen == NULL, EFAULT)
    return __PRT_ProxyRecvFrom(sockfd, buf, len, flags, src_addr, addrlen);
}

ssize_t PRT_ProxySend(int sockfd, const void *buf, size_t len, int flags)
{
    DEFINE_COMMON_RPC_VAR(send)
    ssize_t sret = 0;

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = SEND_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.len = MIN(len, sizeof(req.buf));
    req.flags = flags;
    memcpy_s(req.buf, req.len, buf, req.len);
    payload_size = payload_size - sizeof(req.buf) + req.len;

    RECORD_AT(slot_idx).cb = CONVERT(csret);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

static ssize_t __PRT_ProxySendTo(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    DEFINE_COMMON_RPC_VAR(sendto)
    ssize_t sret = 0;

    CHECK_ARG(addrlen > sizeof(req.buf), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = SENDTO_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.addrlen = addrlen;
    req.len = MIN(len, sizeof(req.buf) - addrlen);
    req.flags = flags;
    memcpy_s(req.buf, addrlen, dest_addr, addrlen);
    memcpy_s(&req.buf[addrlen], req.len, buf, req.len);
    payload_size = payload_size - sizeof(req.buf) + req.len + addrlen;

    RECORD_AT(slot_idx).cb = CONVERT(csret);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    sret = outp.ret;
    free_slot(slot_idx);
    return sret;
}

ssize_t PRT_ProxySendTo(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(buf == NULL, EFAULT)
    if (dest_addr == NULL && addrlen == 0) {
        return PRT_ProxySend(sockfd, buf, len, flags);
    }
    CHECK_ARG(dest_addr == NULL, EDESTADDRREQ)
    CHECK_ARG(addrlen == 0, EFAULT)
    return __PRT_ProxySendTo(sockfd, buf, len, flags, dest_addr, addrlen);
}

int PRT_ProxySetSockOpt(int sockfd, int level, int optname, const void *optval, 
                   socklen_t optlen)
{
    DEFINE_COMMON_RPC_VAR(setsockopt)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(optval == NULL, EFAULT)
    CHECK_ARG(optlen == 0, EINVAL)
    CHECK_ARG(optlen > sizeof(req.optval), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = SETSOCKOPT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.level = level;
    req.optname = optname;
    req.optlen = optlen;
    memcpy_s(req.optval, optlen, optval, optlen);

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyShutdown(int sockfd, int how)
{
    DEFINE_COMMON_RPC_VAR(shutdown)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = SHUTDOWN_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = sockfd;
    req.how = how;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxySocket(int domain, int type, int protocol)
{
    DEFINE_COMMON_RPC_VAR(socket)

    CHECK_INIT()
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = SOCKET_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.domain = domain;
    req.type = type;
    req.protocol = protocol;

    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    free_slot(slot_idx);
    return ret;
}

static int __printf(const char *format, va_list list)
{
    rpc_printf_req_t req;
    int ret = 0;
    int hlen = sizeof(req.func_id) + sizeof(req.len);
    int len = vsnprintf(req.buf, sizeof(req.buf), format, list);
    if (len < 0) {
        return len;
    }

    req.func_id = PRINTF_ID;
    req.len = len;
    
    ret = rpmsg_send(g_ept, &req, len + hlen);

    return ret - hlen;
}

int PRT_ProxyPrintf(const char *format, ...)
{
    int len = 0;
    va_list list;

    CHECK_INIT()
    va_start(list, format);
    len = __printf(format, list);
    va_end(list);

    return len;
}

WEAK_ALIAS(PRT_ProxyOpen, open);
WEAK_ALIAS(PRT_ProxyRead, read);
WEAK_ALIAS(PRT_ProxyWrite, write);
WEAK_ALIAS(PRT_ProxyClose, close);
WEAK_ALIAS(PRT_ProxyLseek, lseek);
WEAK_ALIAS(PRT_ProxyFcntl, fcntl);
WEAK_ALIAS(PRT_ProxyUnlink, unlink);
WEAK_ALIAS(PRT_ProxyFreeAddrInfo, freeaddrinfo);
WEAK_ALIAS(PRT_ProxyGetAddrInfo, getaddrinfo);
WEAK_ALIAS(PRT_ProxyGetHostByAddr, gethostbyaddr);
WEAK_ALIAS(PRT_ProxyGetHostByName, gethostbyname);
WEAK_ALIAS(PRT_ProxyPoll, poll);
WEAK_ALIAS(PRT_ProxyGetPeerName, getpeername);
WEAK_ALIAS(PRT_ProxyGetHostName, gethostname);
WEAK_ALIAS(PRT_ProxyGetSockName, getsockname);
WEAK_ALIAS(PRT_ProxyGetSockOpt, getsockopt);
WEAK_ALIAS(PRT_ProxySelect, select);
WEAK_ALIAS(PRT_ProxyAccept, accept);
WEAK_ALIAS(PRT_ProxyBind, bind);
WEAK_ALIAS(PRT_ProxyConnect, connect);
WEAK_ALIAS(PRT_ProxyListen, listen);
WEAK_ALIAS(PRT_ProxyRecv, recv);
WEAK_ALIAS(PRT_ProxyRecvFrom, recvfrom);
WEAK_ALIAS(PRT_ProxySend, send);
WEAK_ALIAS(PRT_ProxySendTo, sendto);
WEAK_ALIAS(PRT_ProxySetSockOpt, setsockopt);
WEAK_ALIAS(PRT_ProxyShutdown, shutdown);
WEAK_ALIAS(PRT_ProxySocket, socket);

