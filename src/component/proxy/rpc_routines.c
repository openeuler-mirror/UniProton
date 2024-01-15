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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include <openamp/rpmsg.h>
#include <openamp/rpmsg_rpc_client_server.h>

#include "prt_hwi.h"
#include "securec.h"
#include "rpc_internal_common.h"
#include "rpc_internal_model.h"
#include "rpc_client_internal.h"
#include "rpc_err.h"
#include "prt_buildef.h"
#include "prt_queue.h"
#include "prt_proxy_ext.h"

#ifdef LOSCFG_SHELL_MICA_INPUT
#include "../../shell/full/include/shmsg.h"
#endif

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
    outp.super.errnum = 0;

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

#define STDFILE_BASE 1
char *g_printf_buffer = NULL;

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

int PRT_ProxyWriteStdOut(const char *buf, int len);

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
static char *g_s1 = "Hello, UniProton! \r\n";

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
    if (obase != NULL && base->errnum != 0) {
        obase->errnum = base->errnum;
    }
    if (cb != NULL) {
        cb(data, obase);
    }
    /* to clear the flag set in the caller function */
    set_status(idx, STATUS_READY);
}

static fileHandle file2handle(FILE *f)
{
    if (f == stdout) {
        return STDOUT_FILENO + STDFILE_BASE;
    } else if (f == stdin) {
        return STDIN_FILENO + STDFILE_BASE;
    } else if (f == stderr) {
        return STDERR_FILENO + STDFILE_BASE;
    }
    return (fileHandle)f;
}

DEF_CONVERT(common)
{
    DEFINE_CB_VARS(common)

    outp->ret = resp->ret;
}

DEF_CONVERT(fcommon)
{
    DEFINE_CB_VARS(fcommon)

    outp->fhandle = resp->fhandle;
}

DEF_CONVERT(csret)
{
    DEFINE_CB_VARS(csret)

    outp->ret = resp->ret;
}

DEF_CONVERT(read)
{
    DEFINE_CB_VARS(read)
    ssize_t buflen = MIN(((ssize_t)sizeof(resp->buf)), resp->ret);

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
    int buflen = MIN(((int)sizeof(resp->buf)), resp->len);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->len = buflen;
    // todo: set h_errno
}

DEF_CONVERT(getaddrinfo)
{
    DEFINE_CB_VARS(getaddrinfo)
    int buflen = MIN(((int)sizeof(resp->buf)), resp->buflen);

    if (outp->buf != NULL && buflen > 0) {
        memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->cnt = resp->cnt;
    outp->ret = resp->ret;
}

DEF_CONVERT(getpeername)
{
    DEFINE_CB_VARS(getpeername)
    socklen_t buflen = MIN(((socklen_t)sizeof(resp->addr_buf)), resp->addrlen);

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
    socklen_t buflen = MIN(((socklen_t)sizeof(resp->buf)), resp->addrlen);

    if (outp->addrlen != NULL) {
        *outp->addrlen = buflen;
    }
    if (outp->addr != NULL && buflen > 0) {
        memcpy_s(outp->addr, buflen, resp->buf, buflen);
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(accept4)
{
    DEFINE_CB_VARS(accept4)
    socklen_t buflen = MIN(((socklen_t)sizeof(resp->buf)), resp->addrlen);

    if (outp->addrlen != NULL) {
        *outp->addrlen = buflen;
    }
    if (outp->addr != NULL && buflen > 0) {
        (void)memcpy_s(outp->addr, buflen, resp->buf, buflen);
    }
    outp->ret = resp->ret;
}

DEF_CONVERT(gai_strerror)
{
    DEFINE_CB_VARS(gai_strerror)
    if (resp->isNull) {
        return;
    }
    int len = strlen(resp->buf) + 1;
    memcpy_s(outp->buf, len, resp->buf, len);
}

DEF_CONVERT(putchar)
{
    DEFINE_CB_VARS(putchar)
    outp->ret = resp->ret;
}

DEF_CONVERT(if_nameindex)
{
    DEFINE_CB_VARS(if_nameindex)
    outp->cnt = resp->cnt;

    int len = 0;
    for (int i = 0; i < resp->cnt; i++) {
        outp->if_index[i].if_index = resp->if_index[i].if_index;
        len = strlen(resp->if_index[i].if_name) + 1;
        outp->if_index[i].if_name = (char *)malloc(sizeof(char)*len);
        (void)memcpy_s(outp->if_index[i].if_name, MAX_IFNAMEINDEX_LEN, resp->if_index[i].if_name, len);
    }
}

DEF_CONVERT(writev)
{
    DEFINE_CB_VARS(writev)
    outp->ret = resp->ret;
}

DEF_CONVERT(recv)
{
    DEFINE_CB_VARS(recv)
    ssize_t buflen = MIN(((ssize_t)sizeof(resp->buf)), resp->ret);

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

DEF_CONVERT(getdents64)
{
    DEFINE_CB_VARS(getdents64)
    outp->ret = resp->ret;
    outp->pos = resp->pos;

    if (outp->buf == NULL || resp->ret <= 0) {
        return;
    }
    memcpy_s(outp->buf, outp->bufsize, resp->buf, resp->ret);
}

DEF_CONVERT(getwc)
{
    DEFINE_CB_VARS(getwc)

    outp->ret = resp->ret;
}

DEF_CONVERT(stat)
{
    DEFINE_CB_VARS(stat)
    if (outp->statbuf == NULL) {
        return;
    }
    struct stat *statbuf = outp->statbuf;

    outp->ret = resp->ret;

    statbuf->st_dev = resp->st_dev;
    statbuf->st_ino = resp->st_ino;
    statbuf->st_nlink = resp->st_nlink;

    statbuf->st_mode = resp->st_mode;
    statbuf->st_uid = resp->st_uid;
    statbuf->st_gid = resp->st_gid;
    statbuf->st_rdev = resp->st_rdev;
    statbuf->st_size = resp->st_size;
    statbuf->st_blksize = resp->st_blksize;
    statbuf->st_blocks = resp->st_blocks;

    statbuf->st_atim.tv_sec = resp->st_atime_sec;
    statbuf->st_atim.tv_nsec = resp->st_atime_nsec;
    statbuf->st_mtim.tv_sec = resp->st_mtime_sec;
    statbuf->st_mtim.tv_nsec = resp->st_mtime_nsec;
    statbuf->st_ctim.tv_sec = resp->st_ctime_sec;
    statbuf->st_ctim.tv_nsec = resp->st_ctime_nsec;
}

DEF_CONVERT(getcwd)
{
    DEFINE_CB_VARS(getcwd)
    if (outp->buf == NULL || outp->size > MAX_PATH_LEN || resp->isNull) {
        outp->buf = NULL;
        return;
    }
    memcpy_s(outp->buf, outp->size, resp->buf, outp->size);
}

DEF_CONVERT(readlink)
{
    DEFINE_CB_VARS(readlink)
    outp->ret = resp->ret;
    if (outp->buf == NULL || outp->ret <= 0) {
        return;
    }
    memcpy_s(outp->buf, outp->bufsiz, resp->buf, outp->ret);
}

static void resp2outp_fread(void *from, void *to)
{
    rpc_fread_resp_t *resp = (rpc_fread_resp_t *)from;
    rpc_fread_outp_t *outp = (rpc_fread_outp_t *)to;
    if (outp->buf == NULL) {
        return;
    }
    size_t buflen = MIN(sizeof(resp->buf), resp->ret);
    if (buflen > 0) {
        (void)memcpy_s(outp->buf, buflen, resp->buf, buflen);
    }
    outp->ret = resp->ret;
}

static void resp2outp_fgets(void *from, void *to)
{
    rpc_fgets_resp_t *resp = (rpc_fgets_resp_t *)from;
    rpc_fgets_outp_t *outp = (rpc_fgets_outp_t *)to;
    (void)memcpy_s(outp->str, outp->size, resp->str, outp->size);
    outp->isEof = resp->isEof;
}

static void resp2outp_fwrite(void *from, void *to)
{
    rpc_fwrite_resp_t *resp = (rpc_fwrite_resp_t *)from;
    rpc_fwrite_outp_t *outp = (rpc_fwrite_outp_t *)to;
    outp->ret = resp->ret;
}

static void resp2outp_ftello(void *from, void *to)
{
    rpc_ftello_resp_t *resp = (rpc_ftello_resp_t *)from;
    rpc_ftello_outp_t *outp = (rpc_ftello_outp_t *)to;
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
    struct rpmsg_proxy_answer *msg;
    UNUSED(priv);
    UNUSED(src);

    CHECK_COND(data == NULL, RPMSG_ERR_ADDR)
    CHECK_COND(ept == NULL, RPMSG_ERR_INIT)
    msg = (struct rpmsg_proxy_answer *)data;
    dprintf("==(%x,%d)", src, msg->id);

    if (msg->id == 0) {
#ifdef LOSCFG_SHELL_MICA_INPUT
        ShellCB *shellCb = OsGetShellCB();
        if (shellCb == NULL) {
            PRT_ProxyWriteStdOut((void *)g_s1, strlen(g_s1) * sizeof(char));
        } else {
            char c = msg->params[0];
            ShellCmdLineParse(c, (pf_OUTPUT)printf, shellCb);
        }
#else
        PRT_ProxyWriteStdOut((void *)g_s1, strlen(g_s1) * sizeof(char));
#endif
    }

#ifdef OS_SUPPORT_ETHERCAT
    if (is_valid_cmd_func_id(msg->id)) {
        cmd_base_req_t *cmd_req = (cmd_base_req_t *)&(msg->params[0]);
        U32 ret = PRT_QueueWrite(g_cmd_req_queue, cmd_req, len - MAX_FUNC_ID_LEN, 0, OS_QUEUE_NORMAL);
        if (ret != OS_OK) {
            return RPMSG_ERR_NO_MEM;
        }
        return RPMSG_SUCCESS;
    }
#endif

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

    uintptr_t intSave;
    intSave = PRT_HwiLock();
    ret = rpmsg_send(g_ept, data, size);
    PRT_HwiRestore(intSave);

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
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyOpen(const char *filename, int flags, ...)
{
    mode_t mode = 0;

    if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }

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
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return sret;
}

// 无法保证串行化
ssize_t PRT_ProxyReadLoop(int fd, void *buf, size_t count) {
    if (count == 0) {
        return 0;
    }

    if (count <= MAX_STRING_LEN) {
        return PRT_ProxyRead(fd, buf, count);
    } else {
        printf("WARN: read buf too large, using loop read\n");
    }

    size_t lenRemain = count;
    ssize_t lenRead = 0;
    while (lenRemain > 0)
    {
        lenRead = PRT_ProxyRead(fd, buf, lenRemain);
        if (lenRead == 0) {
            break;
        }
        if (lenRead < 0) {
            printf("READ FAIL!!, %lu, %ld, %s\n", lenRemain, lenRead, strerror(errno));
            return lenRead;
        }
        if (lenRemain < lenRead) {
            printf("READ FAIL!!!, %lu, %ld, %s\n", lenRemain, lenRead, strerror(errno));
            lenRemain = 0;
            break;
        }
        lenRemain -= (size_t)lenRead;
        buf += lenRead;
    }
	return count - lenRemain;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    outp.buf = arg;
    payload_size -= sizeof(req.buf);
    if (arg != NULL && req.len > 0) {
        memcpy_s(req.buf, req.len, arg, req.len);
        payload_size += req.len;
    }
    RECORD_AT(slot_idx).cb = CONVERT(ioctl);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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

    RECORD_AT(slot_idx).cb = CONVERT(csret);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    oret = outp.ret;
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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

    errno = outp.super.errnum;
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

    errno = outp.super.errnum;
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

    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
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
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

int PRT_ProxyWriteStdOut(const char *buf, int len)
{
    rpc_printf_req_t req;
    int ret = 0;
    int hlen = offsetof(rpc_printf_req_t, buf);
    if (len <= 0) {
        return len;
    }
    memcpy_s(req.buf, sizeof(req.buf), buf, len);
    len = MIN(len, sizeof(req.buf));
    req.func_id = PRINTF_ID;
    req.len = len;

    uintptr_t intSave;
    intSave = PRT_HwiLock();
    ret = rpmsg_send(g_ept, &req, len + hlen);
    PRT_HwiRestore(intSave);

    return ret - hlen;
}

int PRT_ProxyVprintf(const char *format, va_list list)
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
    
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    ret = rpmsg_send(g_ept, &req, len + hlen);
    PRT_HwiRestore(intSave);

    return ret - hlen;
}

int PRT_ProxyPrintf(const char *format, ...)
{
    int len = 0;
    va_list list;

    CHECK_INIT()
    va_start(list, format);
    len = PRT_ProxyVprintf(format, list);
    va_end(list);

    return len;
}

int PRT_ProxyGetDents64(int fd, char *buf, int len)
{
    DEFINE_COMMON_RPC_VAR(getdents64)

    CHECK_INIT()
    CHECK_ARG(fd < 0 || buf == NULL || len < 0, EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    RECORD_AT(slot_idx).cb = CONVERT(getdents64);
    req.func_id = GETDENTS64_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;

    req.count = len;
    req.pos = -1;
    outp.buf = buf;
    outp.bufsize = len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

FILE *PRT_ProxyFopen(const char *filename, const char *mode)
{
    rpc_fopen_req_t req = {0};
    rpc_fcommon_outp_t outp = {0};

    if (g_ept == NULL || filename == NULL || mode == NULL) {
        errno = EINVAL;
        return NULL;
    }

    int flen = strlen(filename) + 1;
    int mlen = strlen(mode) + 1;
    if (flen > sizeof(req.filename) || mlen > sizeof(req.mode)) {
        errno = EINVAL;
        return NULL;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return NULL;
    }

    outp.super.errnum = 0;
    req.func_id = FOPEN_ID;
    (void)memcpy_s(req.filename, flen, filename, flen);
    (void)memcpy_s(req.mode, mlen, mode, mlen);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(fcommon);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (FILE *)(outp.fhandle);
}

int PRT_ProxyFclose(FILE *f)
{
    rpc_fclose_req_t req = {0};
    rpc_common_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FCLOSE_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

size_t PRT_ProxyFread(void *buffer, size_t size, size_t count, FILE *f)
{
    rpc_fread_req_t req = {0};
    rpc_fread_outp_t outp = {0};
    if (g_ept == NULL || f == NULL || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (size == 0) {
        return 0;
    }

    size_t totalSize = size * count;
    totalSize = MIN(totalSize, MAX_STRING_LEN);
    size_t cnt = totalSize / size;

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    outp.buf = buffer;
    outp.totalSize = totalSize;
    req.func_id = FREAD_ID;
    req.size = size;
    req.count = cnt;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_fread;

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

// 无法保证串行化
size_t PRT_ProxyFreadLoop(void *buffer, size_t size, size_t count, FILE *f) {
    if (size == 0) {
        return 0;
    }
    size_t lenRemain = size * count;
    size_t lenRead = 0;

    if (lenRemain <= MAX_STRING_LEN) {
        return PRT_ProxyFread(buffer, size, count, f);
    } else {
        printf("WARN: fread buf too large, using loop fread\n");
    }

    while (lenRemain > 0)
    {
        lenRead = PRT_ProxyFread(buffer, 1, lenRemain, (FILE *)f);
        if (lenRead == 0) {
            break;
        }
        if (lenRemain < lenRead) {
            printf("FREAD MORE!!!, %lu, %lu, %s\n", lenRemain, lenRead, strerror(errno));
            lenRemain = 0;
            break;
        }
        lenRemain -= lenRead;
        buffer += lenRead;
    }
	return (size * count - lenRemain) / size;
}

size_t PRT_ProxyFwrite(const void *buffer, size_t size, size_t count, FILE *f)
{
    rpc_fwrite_req_t req = {0};
    rpc_fwrite_outp_t outp = {0};
    if (g_ept == NULL || f == NULL || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (size == 0) {
        return 0;
    }

    size_t totalSize = size * count;
    totalSize = MIN(totalSize, MAX_STRING_LEN);
    size_t cnt = totalSize / size;

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FWRITE_ID;
    req.size = size;
    req.count = cnt;
    req.fhandle = file2handle(f);
    (void)memcpy_s(req.buf, totalSize, buffer, totalSize);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_fwrite;

    int ret = wait4resp(slot_idx, &req, sizeof(req) - sizeof(req.buf) + totalSize);
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

// 无法保证串行化
size_t PRT_ProxyFwriteLoop(const void *buffer, size_t size, size_t count, FILE *f) {
    if (size == 0) {
        return 0;
    }
    size_t lenRemain = size * count;
    size_t lenWrite = 0;

    if (lenRemain <= MAX_STRING_LEN) {
        return PRT_ProxyFwrite(buffer, size, count, f);
    } else {
        printf("WARN: fwrite buf too large, using loop fwrite\n");
    }

    while (lenRemain > 0)
    {
        lenWrite = PRT_ProxyFwrite(buffer, 1, lenRemain, (FILE *)f);
        if (lenWrite == 0) {
            printf("fwrite fail, %s\n", strerror(errno));
        }
        if (lenRemain < lenWrite) {
            printf("FWRITE MORE!!!, %lu, %lu, %s\n", lenRemain, lenWrite, strerror(errno));
            lenRemain = 0;
            break;
        }
        lenRemain -= lenWrite;
        buffer += lenWrite;
    }
	return (size * count - lenRemain) / size;
}

FILE *PRT_ProxyFreopen(const char *filename, const char *mode, FILE *f)
{
    rpc_freopen_req_t req = {0};
    rpc_fcommon_outp_t outp = {0};

    if (g_ept == NULL || filename == NULL || mode == NULL || f == NULL) {
        errno = EINVAL;
        return NULL;
    }

    int flen = strlen(filename) + 1;
    int mlen = strlen(mode) + 1;
    if (flen > sizeof(req.filename) || mlen > sizeof(req.mode)) {
        errno = EINVAL;
        return NULL;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return NULL;
    }

    outp.super.errnum = 0;
    req.func_id = FREOPEN_ID;
    (void)memcpy_s(req.filename, flen, filename, flen);
    (void)memcpy_s(req.mode, mlen, mode, mlen);
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(fcommon);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (FILE *)(outp.fhandle);
}

int PRT_ProxyFputs(const char *str, FILE *f)
{
    rpc_fputs_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL || str == NULL) {
        errno = EINVAL;
        return -1;
    }

    int len = strlen(str) + 1;
    if (len > sizeof(req.str)) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FPUTS_ID;
    req.fhandle = file2handle(f);
    (void)memcpy_s(req.str, len, str, len);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

char *PRT_ProxyFgets(char *str, int n, FILE *f)
{
    rpc_fgets_req_t req = {0};
    rpc_fgets_outp_t outp = {0};
    if (g_ept == NULL || f == NULL || str == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (n > MAX_STRING_LEN) {
        errno = ENOMEM;
        return NULL;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return NULL;
    }

    outp.super.errnum = 0;
    outp.size = n;
    outp.str = str;
    req.func_id = FGETS_ID;
    req.fhandle = file2handle(f);
    req.size = n;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_fgets;

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (outp.isEof == 1 ? 0 : str);
}

int PRT_ProxyFeof(FILE *f)
{
    rpc_feof_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FEOF_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

static int __fprintf(FILE *f, const char *format, va_list list)
{
    rpc_fprintf_req_t req;
    int ret = 0;
    int hlen = sizeof(req.func_id) + sizeof(req.len) + sizeof(req.fhandle);
    if (g_printf_buffer == NULL) {
        return -1;
    }
    int len = vsnprintf(g_printf_buffer, PRINTF_BUFFER_LEN, format, list);
    if (len <= 0) {
        return len;
    }

    size_t size = PRT_ProxyFwriteLoop(g_printf_buffer, (size_t)(len), 1, f);
    if (size == 1) {
        return len;
    } else {
        return -1;
    }
}

int PRT_ProxyFprintf(FILE *f, const char *format, ...)
{
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int len = 0;
    va_list list;

    va_start(list, format);
    len = __fprintf(f, format, list);
    va_end(list);

    if (len < 0) {
        printf("fprintf error, caller:%p, errno:%s\n", __builtin_return_address(0), strerror(errno));
    }
    return len;
}

int PRT_ProxyVfprintf(FILE *restrict f, const char *restrict fmt, va_list ap)
{
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int len = __fprintf(f, fmt, ap);
    return len;
}

int PRT_ProxyGetc(FILE *f)
{
    rpc_getc_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = GETC_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyFerror(FILE *f)
{
    rpc_ferror_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FERROR_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyGetcUnlocked(FILE *f)
{
    rpc_getc_unlocked_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = GETC_UNLOCK_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyPclose(FILE *f)
{
    rpc_pclose_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = PCLOSE_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

FILE *PRT_ProxyTmpfile(void)
{
    rpc_tmpfile_req_t req = {0};
    rpc_fcommon_outp_t outp = {0};
    if (g_ept == NULL) {
        errno = EINVAL;
        return NULL;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return NULL;
    }

    outp.super.errnum = 0;
    req.func_id = TMPFILE_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(fcommon);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (FILE *)outp.fhandle;
}

void PRT_ProxyClearerr(FILE *f)
{
    rpc_clearerr_req_t req = {0};
    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return;
    }

    req.func_id = CLEARERR_ID;
    req.fhandle = file2handle(f);

    uintptr_t intSave;
    intSave = PRT_HwiLock();
    (void)rpmsg_send(g_ept, &req, sizeof(req));
    PRT_HwiRestore(intSave);

    return;
}

FILE *PRT_ProxyPopen(const char *cmd, const char *mode)
{
    rpc_popen_req_t req = {0};
    rpc_fcommon_outp_t outp = {0};

    if (g_ept == NULL || cmd == NULL || mode == NULL) {
        errno = EINVAL;
        return NULL;
    }

    int clen = strlen(cmd) + 1;
    int mlen = strlen(mode) + 1;
    if (clen > sizeof(req.cmd) || mlen > sizeof(req.mode)) {
        errno = EINVAL;
        return NULL;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return NULL;
    }

    outp.super.errnum = 0;
    req.func_id = POPEN_ID;
    (void)memcpy_s(req.cmd, clen, cmd, clen);
    (void)memcpy_s(req.mode, mlen, mode, mlen);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(fcommon);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (FILE *)(outp.fhandle);
}

int PRT_ProxyUngetc(int c, FILE *f)
{
    rpc_ungetc_req_t req = {0};
    rpc_common_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = UNGETC_ID;
    req.c = c;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyFseeko(FILE *f, off_t offset, int whence)
{
    rpc_fseeko_req_t req = {0};
    rpc_common_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FSEEKO_ID;
    req.offset = offset;
    req.whence = whence;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

long PRT_ProxyFtello(FILE * f)
{
    rpc_ftello_req_t req = {0};
    rpc_ftello_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FTELLO_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_ftello;

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyFseek(FILE *f, long offset, int whence)
{
    rpc_fseeko_req_t req = {0};
    rpc_common_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FSEEK_ID;
    req.offset = offset;
    req.whence = whence;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

long PRT_ProxyFtell(FILE * f)
{
    rpc_ftello_req_t req = {0};
    rpc_ftello_outp_t outp = {0};

    if (g_ept == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FTELL_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_ftello;

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyRename(const char *old, const char *new)
{
    rpc_rename_req_t req = {0};
    rpc_common_outp_t outp = {0};
    if (old == NULL || new == NULL) {
        errno = EINVAL;
        return -1;
    }

    int oldLen = strlen(old) + 1;
    int newLen = strlen(new) + 1;
    if (oldLen > MAX_FILE_NAME_LEN || newLen > MAX_FILE_NAME_LEN) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = RENAME_ID;
    (void)memcpy_s(req.old, oldLen, old, oldLen);
    (void)memcpy_s(req.new, newLen, new, newLen);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyRemove(const char *path)
{
    rpc_remove_req_t req = {0};
    rpc_common_outp_t outp = {0};

    int len = strlen(path) + 1;
    if (len > MAX_FILE_NAME_LEN) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = REMOVE_ID;
    (void)memcpy_s(req.path, len, path, len);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyMkstemp(char *template)
{
    rpc_mkstemp_req_t req = {0};
    rpc_common_outp_t outp = {0};

    int len = strlen(template) + 1;
    if (len > MAX_FILE_NAME_LEN) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = MKSTMP_ID;
    (void)memcpy_s(req.tmp, len, template, len);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return -1;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyFflush(FILE *f)
{
    DEFINE_COMMON_RPC_VAR(fflush)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = FFLUSH_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

wint_t PRT_ProxyGetwc(FILE *f)
{
    DEFINE_COMMON_RPC_VAR(getwc)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = GETWC_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(getwc);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

wint_t PRT_ProxyPutwc(wchar_t wc, FILE *f)
{
    DEFINE_COMMON_RPC_VAR(putwc)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = PUTWC_ID;
    req.wc = wc;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(getwc);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyPutc(int c, FILE *f)
{
    DEFINE_COMMON_RPC_VAR(putc)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = PUTC_ID;
    req.c = c;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

wint_t PRT_ProxyUngetwc(wint_t wc, FILE *f)
{
    DEFINE_COMMON_RPC_VAR(ungetwc)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = UNGETWC_ID;
    req.wc = wc;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(getwc);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

int PRT_ProxyStat(const char *restrict pathname, struct stat *restrict statbuf)
{
    DEFINE_COMMON_RPC_VAR(stat)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL || statbuf == NULL, EINVAL)
    int len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.path), ENAMETOOLONG)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    outp.statbuf = statbuf;

    req.func_id = STAT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    (void)memcpy_s(req.path, MAX_PATH_LEN, pathname, len);
    RECORD_AT(slot_idx).cb = CONVERT(stat);
    payload_size = payload_size - sizeof(req.path) + len;

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyLstat(const char *restrict pathname, struct stat *restrict statbuf)
{
    DEFINE_COMMON_RPC_VAR(stat)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL || statbuf == NULL, EINVAL)
    int len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.path), ENAMETOOLONG)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    outp.statbuf = statbuf;

    req.func_id = LSTAT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    (void)memcpy_s(req.path, MAX_PATH_LEN, pathname, len);
    RECORD_AT(slot_idx).cb = CONVERT(stat);
    payload_size = payload_size - sizeof(req.path) + len;

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

/* can't malloc buf of size exceed MAX_PATH_LEN */
char *PRT_ProxyGetcwd(char *buf, size_t size)
{
    DEFINE_COMMON_RPC_VAR(getcwd)

    CHECK_RET_NULL(g_ept == NULL)
    CHECK_AND_SET_ERRNO(buf != NULL && size == 0, NULL, EINVAL)
    int isNull = 0;

    slot_idx = new_slot(&outp);
    CHECK_RET_NULL(slot_idx < 0)
    if ((buf == NULL && size == 0) || size > MAX_PATH_LEN) {
        size = MAX_PATH_LEN;
    }

    if (buf == NULL) {
        outp.buf = malloc(size);
        if (outp.buf == NULL) {
            errno = ENOMEM;
            goto getcwd_exit;
        }
        isNull = 1;
    } else {
        outp.buf = buf;
    }
    outp.size = size;

    req.func_id = GETCWD_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.size = size;
    RECORD_AT(slot_idx).cb = CONVERT(getcwd);

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET_NULL(ret < 0)
    errno = outp.super.errnum;
    if (outp.super.errnum == ERANGE && size == MAX_PATH_LEN) {
        errno = ENAMETOOLONG;
    }
getcwd_exit:
    free_slot(slot_idx);
    return outp.buf;
}

int PRT_ProxyFstat(int fd, struct stat *restrict statbuf)
{
    DEFINE_COMMON_RPC_VAR(fstat)

    CHECK_INIT()
    CHECK_ARG(statbuf == NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    outp.statbuf = statbuf;

    req.func_id = FSTAT_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    RECORD_AT(slot_idx).cb = CONVERT(stat);

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

FILE *PRT_ProxyFdopen(int fd, const char *mode)
{
    DEFINE_COMMON_RPC_VAR(fdopen)
    CHECK_RET_NULL(g_ept == NULL)
    CHECK_AND_SET_ERRNO(mode == NULL, NULL, EINVAL)
    int mlen = strlen(mode) + 1;
    CHECK_AND_SET_ERRNO(mlen > sizeof(req.mode), NULL, EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_AND_SET_ERRNO(slot_idx < 0, NULL, EBUSY)

    req.func_id = FDOPEN_ID;
    req.fd = fd;
    (void)memcpy_s(req.mode, mlen, mode, mlen);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(fcommon);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET_NULL(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);

    return (FILE *)(outp.fhandle);
}

int PRT_ProxyFileno(FILE *f)
{
    DEFINE_COMMON_RPC_VAR(fileno)

    CHECK_INIT()
    CHECK_ARG(f == NULL, EINVAL)
    fileHandle fhandle = file2handle(f);
    if (fhandle >= STDIN_FILENO + STDFILE_BASE &&
        fhandle <= STDERR_FILENO + STDFILE_BASE) {
        return fhandle - STDFILE_BASE;
    }
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    req.func_id = FILENO_ID;
    req.fhandle = fhandle;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxySetvbuf(FILE *f, char *buf, int mode, size_t size)
{
    DEFINE_COMMON_RPC_VAR(setvbuf)
    UNUSED(buf);
    UNUSED(size);

    CHECK_INIT()
    CHECK_ARG(f == NULL || mode != _IONBF, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    req.func_id = SETVBUF_ID;
    req.fhandle = file2handle(f);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);

    ret = wait4resp(slot_idx, &req, sizeof(req));
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxySystem(const char *command)
{
    DEFINE_COMMON_RPC_VAR(system)

    CHECK_INIT()
    CHECK_ARG(command == NULL, EINVAL)
    size_t len = strlen(command) + 1;
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.buf, sizeof(req.buf), command, len);
    req.func_id = SYSTEM_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.buf) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

ssize_t PRT_ProxyReadLink(const char *pathname, char *buf, size_t bufsiz)
{
    DEFINE_COMMON_RPC_VAR(readlink)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL || buf == NULL || !bufsiz, EINVAL)
    size_t len = strlen(pathname) + 1;
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.pathname, sizeof(req.pathname), pathname, len);
    req.func_id = READLINK_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.bufsiz = MIN(MAX_STRING_LEN, bufsiz);
    RECORD_AT(slot_idx).cb = CONVERT(readlink);
    payload_size = payload_size - sizeof(req.pathname) + len;
    outp.buf = buf;
    outp.bufsiz = bufsiz;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyAccess(const char *pathname, int mode)
{
    DEFINE_COMMON_RPC_VAR(access)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL, EINVAL)
    size_t len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.pathname), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.pathname, sizeof(req.pathname), pathname, len);
    req.func_id = ACCESS_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.mode = mode;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.pathname) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyDup2(int oldfd, int newfd)
{
    DEFINE_COMMON_RPC_VAR(dup2)

    CHECK_INIT()
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    req.func_id = DUP2_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.oldfd = oldfd;
    req.newfd = newfd;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyMkfifo(const char *pathname, mode_t mode)
{
    DEFINE_COMMON_RPC_VAR(mkfifo)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL, EINVAL)
    size_t len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.pathname), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.pathname, sizeof(req.pathname), pathname, len);
    req.func_id = MKFIFO_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.mode = mode;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.pathname) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyChmod(const char *pathname, mode_t mode)
{
    DEFINE_COMMON_RPC_VAR(chmod)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL, EINVAL)
    size_t len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.pathname), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.pathname, sizeof(req.pathname), pathname, len);
    req.func_id = CHMOD_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.mode = mode;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.pathname) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyChdir(const char *path)
{
    DEFINE_COMMON_RPC_VAR(chdir)

    CHECK_INIT()
    CHECK_ARG(path == NULL, EINVAL)
    size_t len = strlen(path) + 1;
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.buf, sizeof(req.buf), path, len);
    req.func_id = CHDIR_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.buf) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyMkdir(const char *pathname, mode_t mode)
{
    DEFINE_COMMON_RPC_VAR(mkdir)

    CHECK_INIT()
    CHECK_ARG(pathname == NULL, EINVAL)
    size_t len = strlen(pathname) + 1;
    CHECK_ARG(len > sizeof(req.pathname), ENAMETOOLONG)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.pathname, sizeof(req.pathname), pathname, len);
    req.func_id = MKDIR_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.mode = mode;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.pathname) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

int PRT_ProxyRmdir(const char *path)
{
    DEFINE_COMMON_RPC_VAR(rmdir)

    CHECK_INIT()
    CHECK_ARG(path == NULL, EINVAL)
    size_t len = strlen(path) + 1;
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)
    (void)memcpy_s(&req.buf, sizeof(req.buf), path, len);
    req.func_id = RMDIR_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(common);
    payload_size = payload_size - sizeof(req.buf) + len;
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret < 0)
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return outp.ret;
}

DEF_CONVERT(pipe)
{
    DEFINE_CB_VARS(pipe)

    outp->fd[0] = resp->fd[0];
    outp->fd[1] = resp->fd[1];
    outp->ret = resp->ret;
}

int PRT_ProxyPipe(int fd[2])
{
    DEFINE_COMMON_RPC_VAR(pipe)

    CHECK_INIT()
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = PIPE_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;

    RECORD_AT(slot_idx).cb = CONVERT(pipe);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    fd[0] = outp.fd[0];
    fd[1] = outp.fd[1];
    ret = outp.ret;
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

static void resp2outp_fscanfx(void *from, void *to)
{
    rpc_fscanfx_resp_t *resp = (rpc_fscanfx_resp_t *)from;
    rpc_fscanfx_outp_t *outp = (rpc_fscanfx_outp_t *)to;

    outp->data = resp->data;
    outp->ret = resp->ret;
}

int PRT_ProxyVfscanfx(FILE *f, const char *fmt, va_list ap)
{
    rpc_fscanfx_req_t req = {0};
    rpc_fscanfx_outp_t outp = {0};

    if (g_ept == NULL || f == NULL || fmt == NULL) {
        errno = EINVAL;
        return -1;
    }

    int len = strlen(fmt) + 1;
    if (len > sizeof(req.fmt)) {
        errno = EINVAL;
        return -1;
    }

    int slot_idx = new_slot(&outp);
    if (slot_idx < 0) {
        errno = ETXTBSY;
        return -1;
    }

    outp.super.errnum = 0;
    req.func_id = FSCANFX_ID;
    req.fhandle = file2handle(f);
    (void)memcpy_s(req.fmt, len, fmt, len);
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = resp2outp_fscanfx;
    int ret = wait4resp(slot_idx, &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    uint64_t *data_ptr = va_arg(ap, uint64_t*);
    *data_ptr = outp.data;

    return outp.ret;
}

int PRT_ProxyFscanfx(FILE *f, const char *fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = PRT_ProxyVfscanfx(f, fmt, ap);
    va_end(ap);
    return ret;
}

struct if_nameindex *PRT_ProxyIfNameIndex()
{
    DEFINE_COMMON_RPC_VAR(if_nameindex)
    outp.if_index = (struct if_nameindex *)malloc(sizeof(struct if_nameindex)*MAX_IFNAMEINDEX_SIZE);
    CHECK_RET_NULL(g_ept == NULL)
    CHECK_RET_NULL(outp.if_index == NULL)
    memset_s(outp.if_index, sizeof(struct if_nameindex)*MAX_IFNAMEINDEX_SIZE,
        0, sizeof(struct if_nameindex)*MAX_IFNAMEINDEX_SIZE);

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = IFNAMEINDEX_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(if_nameindex);

    ret = wait4resp(slot_idx, &req, payload_size);
    if (ret < 0) {
        return NULL;
    }
    errno = outp.super.errnum;
    free_slot(slot_idx);

    if (outp.cnt == 0) {
        return NULL;
    }

    return outp.if_index;
}

int PRT_ProxyPutChar(int ch)
{
    DEFINE_COMMON_RPC_VAR(putchar)
    CHECK_INIT()

    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = PUTCHAR_ID;
    req.ch = ch;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(putchar);

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.ret;
}

const char *PRT_ProxyGaiStrError(int error)
{
    DEFINE_COMMON_RPC_VAR(gai_strerror)
    CHECK_RET_NULL(g_ept == NULL)
    CHECK_AND_SET_ERRNO(error > 0, NULL, EINVAL)

    slot_idx = new_slot(&outp);
    CHECK_RET_NULL(slot_idx < 0)

    req.func_id = GAISTRERROR_ID;
    req.error = error;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    RECORD_AT(slot_idx).cb = CONVERT(gai_strerror);

    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET_NULL(ret < 0)

    errno = outp.super.errnum;
    free_slot(slot_idx);

    return outp.buf;
}

int PRT_ProxyAccept4(int sockfd, struct sockaddr *addr,socklen_t *addrlen, int flags)
{
    DEFINE_COMMON_RPC_VAR(accept4)

    CHECK_INIT()
    CHECK_ARG(sockfd < 0, EBADF)
    CHECK_ARG(addr != NULL && addrlen == NULL , EFAULT)
    CHECK_ARG(addrlen != NULL && *addrlen > sizeof(req.addr_buf), EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = ACCEPT4_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.sockfd = sockfd;
    req.flags = flags;
    
    if (addr != NULL && addrlen != NULL) {
        req.addrlen = *addrlen;
        (void)memcpy_s(req.addr_buf, req.addrlen, addr, req.addrlen);
    } else {
        req.addrlen = 0;
        payload_size -= sizeof(req.addr_buf);
    }

    outp.addr = addr;
    outp.addrlen = addrlen;

    RECORD_AT(slot_idx).cb = CONVERT(accept4);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

ssize_t PRT_ProxyWritev(int fd, const struct iovec *iov, int iovcnt)
{
    DEFINE_COMMON_RPC_VAR(writev)

    CHECK_INIT()
    CHECK_ARG(fd < 0, EBADF)
    CHECK_ARG(iov == NULL , EFAULT)
    CHECK_ARG(iovcnt == 0, EINVAL)
    CHECK_ARG(iovcnt > MAX_IOV_SIZE, EINVAL)
    slot_idx = new_slot(&outp);
    CHECK_RET(slot_idx)

    req.func_id = WRITEV_ID;
    req.trace_id = RECORD_AT(slot_idx).trace_id;
    req.fd = fd;
    req.iovcnt = iovcnt;

    for (int i = 0; i < iovcnt; i++) {
        memcpy_s(req.buf[i].iov, MAX_IOV_LEN, (char *)iov->iov_base, iov->iov_len);
        req.buf[i].len = iov->iov_len;
        iov++;
    }

    RECORD_AT(slot_idx).cb = CONVERT(writev);
    ret = wait4resp(slot_idx, &req, payload_size);
    CHECK_RET(ret)

    ret = outp.ret;
    errno = outp.super.errnum;
    free_slot(slot_idx);
    return ret;
}

#if defined(OS_OPTION_PROXY) && !defined(OS_OPTION_PROXY_NO_API)
WEAK_ALIAS(PRT_ProxyOpen, open);
WEAK_ALIAS(PRT_ProxyReadLoop, read);
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
WEAK_ALIAS(PRT_ProxyFopen, fopen);
WEAK_ALIAS(PRT_ProxyFclose, fclose);
WEAK_ALIAS(PRT_ProxyFreadLoop, fread);
WEAK_ALIAS(PRT_ProxyFwriteLoop, fwrite);
WEAK_ALIAS(PRT_ProxyFreopen, freopen);
WEAK_ALIAS(PRT_ProxyFputs, fputs);
WEAK_ALIAS(PRT_ProxyFgets, fgets);
WEAK_ALIAS(PRT_ProxyFeof, feof);
WEAK_ALIAS(PRT_ProxyFprintf, fprintf);
WEAK_ALIAS(PRT_ProxyGetc, getc);
WEAK_ALIAS(PRT_ProxyFerror, ferror);
WEAK_ALIAS(PRT_ProxyGetcUnlocked, getc_unlocked);
WEAK_ALIAS(PRT_ProxyPclose, pclose);
WEAK_ALIAS(PRT_ProxyTmpfile, tmpfile);
WEAK_ALIAS(PRT_ProxyClearerr, clearerr);
WEAK_ALIAS(PRT_ProxyPopen, popen);
WEAK_ALIAS(PRT_ProxyUngetc, ungetc);
WEAK_ALIAS(PRT_ProxyFseeko, fseeko);
WEAK_ALIAS(PRT_ProxyFtello, ftello);
WEAK_ALIAS(PRT_ProxyFseek, fseek);
WEAK_ALIAS(PRT_ProxyFtell, ftell);
WEAK_ALIAS(PRT_ProxyRename, rename);
WEAK_ALIAS(PRT_ProxyRemove, remove);
WEAK_ALIAS(PRT_ProxyMkstemp, mkstemp);
WEAK_ALIAS(PRT_ProxyGetwc, getwc);
WEAK_ALIAS(PRT_ProxyPutwc, putwc);
WEAK_ALIAS(PRT_ProxyPutc, putc);
WEAK_ALIAS(PRT_ProxyPutc, fputc);
WEAK_ALIAS(PRT_ProxyUngetwc, ungetwc);
WEAK_ALIAS(PRT_ProxyFflush, fflush);
WEAK_ALIAS(PRT_ProxyStat, stat);
WEAK_ALIAS(PRT_ProxyGetcwd, getcwd);
WEAK_ALIAS(PRT_ProxyVfprintf, vfprintf);
WEAK_ALIAS(PRT_ProxyLstat, lstat);
WEAK_ALIAS(PRT_ProxyFstat, fstat);
WEAK_ALIAS(PRT_ProxyFileno, fileno);
WEAK_ALIAS(PRT_ProxyFdopen, fdopen);
WEAK_ALIAS(PRT_ProxySetvbuf, setvbuf);
WEAK_ALIAS(PRT_ProxyVprintf, vprintf);
WEAK_ALIAS(PRT_ProxyReadLink, readlink);
WEAK_ALIAS(PRT_ProxySystem, system);
WEAK_ALIAS(PRT_ProxyAccess, access);
WEAK_ALIAS(PRT_ProxyDup2, dup2);
WEAK_ALIAS(PRT_ProxyMkfifo, mkfifo);
WEAK_ALIAS(PRT_ProxyChmod, chmod);
WEAK_ALIAS(PRT_ProxyChdir, chdir);
WEAK_ALIAS(PRT_ProxyMkdir, mkdir);
WEAK_ALIAS(PRT_ProxyRmdir, rmdir);
WEAK_ALIAS(PRT_ProxyPipe, pipe);
WEAK_ALIAS(PRT_ProxyFscanfx, fscanf);
WEAK_ALIAS(PRT_ProxyIfNameIndex, if_nameindex);
WEAK_ALIAS(PRT_ProxyPutChar, putchar);
WEAK_ALIAS(PRT_ProxyGaiStrError, gai_strerror);
WEAK_ALIAS(PRT_ProxyAccept4, accept4);
WEAK_ALIAS(PRT_ProxyWritev, writev);
#endif
