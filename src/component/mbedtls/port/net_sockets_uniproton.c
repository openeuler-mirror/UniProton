#include "mbedtls/net_sockets.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "prt_task.h"

#if defined(MBEDTLS_NET_C)

static int parse_port(const char *port)
{
    char *end = NULL;
    long value;

    if (port == NULL) {
        return -1;
    }

    value = strtol(port, &end, 10);
    if (end == port || *end != '\0' || value <= 0 || value > 65535) {
        return -1;
    }

    return (int)value;
}

void mbedtls_net_init(mbedtls_net_context *ctx)
{
    ctx->fd = -1;
}

int mbedtls_net_bind(mbedtls_net_context *ctx, const char *bind_ip, const char *port, int proto)
{
    struct sockaddr_in addr;
    int fd;
    int reuse = 1;
    int port_num = parse_port(port);

    if (port_num < 0 || (proto != MBEDTLS_NET_PROTO_UDP && proto != MBEDTLS_NET_PROTO_TCP)) {
        return MBEDTLS_ERR_NET_BAD_INPUT_DATA;
    }

    fd = socket(AF_INET, proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (fd < 0) {
        return MBEDTLS_ERR_NET_SOCKET_FAILED;
    }

    (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port_num);
    addr.sin_addr.s_addr = bind_ip == NULL ? htonl(INADDR_ANY) : inet_addr(bind_ip);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return MBEDTLS_ERR_NET_BIND_FAILED;
    }

    if (proto == MBEDTLS_NET_PROTO_TCP && listen(fd, MBEDTLS_NET_LISTEN_BACKLOG) != 0) {
        close(fd);
        return MBEDTLS_ERR_NET_LISTEN_FAILED;
    }

    ctx->fd = fd;
    return 0;
}

int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host, const char *port, int proto)
{
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL;
    struct addrinfo *cur;
    int ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &addr_list) != 0) {
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        ctx->fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (ctx->fd < 0) {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }
        if (connect(ctx->fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            ret = 0;
            break;
        }
        close(ctx->fd);
        ctx->fd = -1;
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo(addr_list);
    return ret;
}

int mbedtls_net_accept(mbedtls_net_context *bind_ctx, mbedtls_net_context *client_ctx,
    void *client_ip, size_t buf_size, size_t *ip_len)
{
    struct sockaddr_in client_addr;
    struct sockaddr_in local_addr;
    socklen_t client_len = sizeof(client_addr);
    socklen_t local_len = sizeof(local_addr);
    char peek;
    int ret;
    int reuse = 1;

    ret = recvfrom(bind_ctx->fd, &peek, sizeof(peek), MSG_PEEK,
        (struct sockaddr *)&client_addr, &client_len);
    if (ret < 0) {
        return MBEDTLS_ERR_NET_ACCEPT_FAILED;
    }

    if (connect(bind_ctx->fd, (struct sockaddr *)&client_addr, client_len) != 0) {
        return MBEDTLS_ERR_NET_ACCEPT_FAILED;
    }

    client_ctx->fd = bind_ctx->fd;
    bind_ctx->fd = -1;

    if (getsockname(client_ctx->fd, (struct sockaddr *)&local_addr, &local_len) != 0) {
        return MBEDTLS_ERR_NET_SOCKET_FAILED;
    }

    bind_ctx->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (bind_ctx->fd < 0) {
        return MBEDTLS_ERR_NET_SOCKET_FAILED;
    }
    (void)setsockopt(bind_ctx->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (bind(bind_ctx->fd, (struct sockaddr *)&local_addr, local_len) != 0) {
        close(bind_ctx->fd);
        bind_ctx->fd = -1;
        return MBEDTLS_ERR_NET_BIND_FAILED;
    }

    if (client_ip != NULL) {
        if (buf_size < sizeof(client_addr.sin_addr.s_addr)) {
            return MBEDTLS_ERR_NET_BUFFER_TOO_SMALL;
        }
        memcpy(client_ip, &client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr));
        if (ip_len != NULL) {
            *ip_len = sizeof(client_addr.sin_addr.s_addr);
        }
    }

    return 0;
}

int mbedtls_net_poll(mbedtls_net_context *ctx, uint32_t rw, uint32_t timeout)
{
    fd_set read_fds;
    fd_set write_fds;
    struct timeval tv;
    struct timeval *tv_ptr = NULL;
    int ret;

    if (ctx->fd < 0) {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    if ((rw & MBEDTLS_NET_POLL_READ) != 0) {
        FD_SET(ctx->fd, &read_fds);
    }
    if ((rw & MBEDTLS_NET_POLL_WRITE) != 0) {
        FD_SET(ctx->fd, &write_fds);
    }
    if (timeout != (uint32_t)-1) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        tv_ptr = &tv;
    }

    ret = select(ctx->fd + 1, &read_fds, &write_fds, NULL, tv_ptr);
    if (ret < 0) {
        return MBEDTLS_ERR_NET_POLL_FAILED;
    }

    ret = 0;
    if (FD_ISSET(ctx->fd, &read_fds)) {
        ret |= MBEDTLS_NET_POLL_READ;
    }
    if (FD_ISSET(ctx->fd, &write_fds)) {
        ret |= MBEDTLS_NET_POLL_WRITE;
    }

    return ret;
}

int mbedtls_net_set_block(mbedtls_net_context *ctx)
{
    (void)ctx;
    return 0;
}

int mbedtls_net_set_nonblock(mbedtls_net_context *ctx)
{
    (void)ctx;
    return 0;
}

void mbedtls_net_usleep(unsigned long usec)
{
    uint32_t ticks = (uint32_t)((usec + 9999UL) / 10000UL);
    if (ticks == 0) {
        ticks = 1;
    }
    PRT_TaskDelay(ticks);
}

int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int fd = ((mbedtls_net_context *)ctx)->fd;
    int ret;

    if (fd < 0) {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    ret = (int)recv(fd, buf, len, 0);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ret;
}

int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    mbedtls_net_context *net_ctx = (mbedtls_net_context *)ctx;
    int ret = mbedtls_net_poll(net_ctx, MBEDTLS_NET_POLL_READ, timeout);

    if (ret < 0) {
        return ret;
    }
    if ((ret & MBEDTLS_NET_POLL_READ) == 0) {
        return MBEDTLS_ERR_SSL_TIMEOUT;
    }

    return mbedtls_net_recv(ctx, buf, len);
}

int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int fd = ((mbedtls_net_context *)ctx)->fd;
    int ret;

    if (fd < 0) {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    ret = (int)send(fd, buf, len, 0);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    return ret;
}

void mbedtls_net_free(mbedtls_net_context *ctx)
{
    if (ctx == NULL || ctx->fd < 0) {
        return;
    }
    (void)shutdown(ctx->fd, SHUT_RDWR);
    (void)close(ctx->fd);
    ctx->fd = -1;
}

#endif /* MBEDTLS_NET_C */
