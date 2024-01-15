#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/uio.h>
#include <dirent.h>
#include "prt_proxy_ext.h"
#include <stdio.h>
#include "pthread.h"

#define FD_SETSIZE 1024
#define dprintf(format, ...) PRT_ProxyPrintf(format, ##__VA_ARGS__)

#define CHECK_EXPECT(name, e, a) do{                                                  \
    __typeof__(e) _e = e;                                                             \
    __typeof__(a) _a = a;                                                             \
    if ((_e) != (_a)) {                                                               \
        dprintf("check " #name " failed, expect: %d, actual: %d\n", (_e), (_a));      \
        return -1;                                                                    \
    }                                                                                 \
} while(0) ;

#define REDEF_O_CREAT   0000100
#define REDEF_O_EXCL    0000200
#define REDEF_O_RDONLY  0000000
#define REDEF_O_WRONLY  0000001
#define REDEF_O_RDWR    0000002
#define REDEF_O_APPEND  0002000
#define REDEF_O_ACCMODE 0000003

static int test_fs_posix1()
{
    char *fname = "/tmp/remote.file";
    char *str = "A Test string being written to file..";
    int fd = 0;
    int ret = 0;
    off_t off = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fd = open(fname, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    if (fd < 0) {
        dprintf("\nUP>Open file '%s' fail, ret: %d\r\n", fname, fd);
        return fd;
    }
    dprintf("\nUP>Opened file '%s' with fd = %d\r\n", fname, fd);

    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        dprintf("\nUP>write fail, ret: %d\r\n", ret);
        goto close_file;
    }
    dprintf("\nUP>Wrote to fd = %d, size = %d, content = %s\r\n", fd, ret, str);

    off = lseek(fd, 0, SEEK_SET);
    if (off < 0) {
        dprintf("\nUP>lseek fail, ret %d\r\n", ret);
        goto close_file;
    }
    ret = read(fd, rbuff, sizeof(rbuff));
    if (ret < 0) {
        dprintf("\nUP>Read from fd = %d, failed ret %d\r\n", fd, ret);
        goto close_file;
    }
    rbuff[ret] = 0;
    dprintf("\nUP>Read from fd = %d, size = %d, "
    "printing contents below .. %s\r\n", fd, ret, rbuff);

close_file:
    close(fd);
    dprintf("\nUP>Closed fd = %d\r\n", fd);
    return ret >= 0 ? 0 : -1;
}

static int test_fs_posix2()
{
    int ret = 0;
    char *fname = "/tmp/remote.file3";
    ret = unlink(fname);
    if (ret >= 0) {
        dprintf("unlink failed\n");
        return -1;
    }
    ret = fcntl(-1, 0, 1);
    if (ret >= 0) {
        dprintf("fcntl failed\n");
        return -1;
    }
    ret = PRT_ProxyIoctl(-1, 0, NULL, 0);
    if (ret >= 0) {
        dprintf("ioctl failed\n");
        return -1;
    }
    return 0;
}

static int test_fs_posix3()
{
    char *tmpfname = "/tmp/tmpXXXXXX";
    char *str = "A Test string being written to file..";
    int fd = 0;
    int ret = 0;
    off_t off = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fd = mkstemp(tmpfname);
    if (fd < 0) {
        dprintf("\nUP>mkstemp file '%s' fail, ret: %d\r\n", tmpfname, fd);
        return fd;
    }
    dprintf("\nUP>mkstemp file '%s' with fd = %d\r\n", tmpfname, fd);

    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        dprintf("\nUP>write fail, ret: %d\r\n", ret);
        goto close_file;
    }
    dprintf("\nUP>Wrote to fd = %d, size = %d, content = %s\r\n", fd, ret, str);

    off = lseek(fd, 0, SEEK_SET);
    if (off < 0) {
        dprintf("\nUP>lseek fail, ret %d\r\n", ret);
        goto close_file;
    }
    ret = read(fd, rbuff, sizeof(rbuff));
    if (ret < 0) {
        dprintf("\nUP>Read from fd = %d, failed ret %d\r\n", fd, ret);
        goto close_file;
    }
    rbuff[ret] = 0;
    dprintf("\nUP>Read from fd = %d, size = %d, "
    "printing contents below .. %s\r\n", fd, ret, rbuff);

close_file:
    close(fd);
    dprintf("\nUP>Closed fd = %d\r\n", fd);
    return ret >= 0 ? 0 : -1;
}

#define BACKLOG         5
#define SV_SOCK_PATH    "/tmp/us_xfr"
#define BUF_SIZE        100
#define STDOUT_FILENO   1
#define STDIN_FILENO    0

#ifndef PF_LOCAL
#define PF_LOCAL    1    /* Local to host (pipes and file-domain).  */
#define PF_UNIX        PF_LOCAL /* POSIX name for PF_LOCAL.  */
#define AF_UNIX     PF_UNIX

#define PF_UNSPEC    0    /* Unspecified.  */
#define AF_UNSPEC    PF_UNSPEC
#define AI_PASSIVE    0x0001    /* Socket address is intended for `bind'.  */

#define PF_INET        2    /* IP protocol family.  */
#define AF_INET        PF_INET
#endif

/* server */
static int test_domain_server()
{
    int sfd = 0;
    int ret = 0;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) {
        dprintf("socket, %d", sfd);
        return -1;
    }

    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1) {
        dprintf("Server socket path too long: %s", SV_SOCK_PATH);
        return -1;
    }

    if (unlink(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        dprintf("remove-%s", SV_SOCK_PATH);
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(SV_SOCK_PATH));
    ret = bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        dprintf("bind(%d)", ret);
        return -1;
    }
    ret = listen(sfd, BACKLOG);
    if (ret < 0) {
        dprintf("listen(%d)", ret);
        return -1;
    }

    for (int i = 0; i < 3; i++) {          /* Handle client connections iteratively */
        /* Accept a connection. The connection is returned on a new
           socket, 'cfd'; the listening socket ('sfd') remains open
           and can be used to accept further connections. */
        ssize_t numRead = 0;
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            dprintf("accept(%d)", cfd);
            return -1;
        }
        while ((numRead = recv(cfd, buf, BUF_SIZE, 0)) > 0) {
            buf[numRead] = '\0';
            dprintf("recv: %s\n", buf);
        }
        if (numRead < -1) {
            dprintf("read(%ld)", numRead);
            return -1;
        }
        ret = close(cfd);
        if (ret < 0) {
            dprintf("close(%d)", ret);
            return -1;
        }
    }
    return 0;
}

/* client */
static int test_domain_client()
{
    int sfd = 0;
    ssize_t sret = 0;
    char *str = "hello server\n";
    char *eofstr = "EOF";
    struct sockaddr_un addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);      /* Create client socket */
    if (sfd < 0) {
        dprintf("socket(%d)", sfd);
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(SV_SOCK_PATH));

    sret = connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un));
    if (sret < 0) {
        dprintf("connect(%ld)", sret);
        return -1;
    }

    sret = send(sfd, str, strlen(str), 0);
    dprintf("send ret %ld\n", sret);
    if (sret < 0) {
        (void)shutdown(sfd, SHUT_RDWR);
        return sret;
    }
    sret = send(sfd, eofstr, strlen(eofstr), 0);
    dprintf("send ret %ld\n", sret);
    if (sret < 0) {
        (void)shutdown(sfd, SHUT_RDWR);
        return sret;
    }
    sret = shutdown(sfd, SHUT_RDWR);
    dprintf("shutdown ret %ld\n", sret);
    if (sret < 0) {
        return sret;
    }
    return 0;        /* Closes our socket; server sees EOF */
}


static void dprintfaddrinfo (struct addrinfo *p)
{
    while (p != NULL) {
        dprintf("%d, %d, %d, %d, %d, %s\n", p->ai_flags, p->ai_family,
                p->ai_socktype, p->ai_protocol, p->ai_addrlen, p->ai_canonname);
        p = p->ai_next;
    }
}

/* getaddrinfo */
static int test_getaddrinfo() {
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    int ret = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    ret = getaddrinfo(NULL, "22", &hints, &result);
    dprintf("ret %d\n", ret);
    if (ret < 0) {
        return ret;
    }
    dprintfaddrinfo(result);
    freeaddrinfo(result);
    return 0;
}

/* paddr: print the IP address in a standard decimal dotted format */
static void paddr(unsigned char *a)
{
    dprintf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}
static void print_host(struct hostent *hp)
{
    int i;

    if (hp == NULL) {
        return;
    }
    dprintf("name:%s, %d\n", hp->h_name, hp->h_length);
    for (i = 0; hp->h_addr_list[i] != 0; i++) {
        paddr((unsigned char*) hp->h_addr_list[i]);
    }
    for (i = 0; hp->h_aliases[i] != 0; i++) {
        dprintf("alias:%s\n", hp->h_aliases[i]);
    }
}


static void build_addr(struct in_addr *addr, int b1, int b2,
                       int b3, int b4)
{
    char *cs = (char *)addr;

    cs[0] = b1 & 0xff;
    cs[1] = b2 & 0xff;
    cs[2] = b3 & 0xff;
    cs[3] = b4 & 0xff;
}

/* gethostbyaddr */
static int test_gethostbyaddr() 
{
    struct hostent *hp;
    struct in_addr sin_addr;

    build_addr(&sin_addr, 127, 0, 0, 1);
    hp = gethostbyaddr(&sin_addr, 4, AF_INET);
    if (hp == NULL) {
        dprintf("gethostbyaddr failed\n");
        return -1;
    }
    print_host(hp);
    free(hp);
    return 0;
}

/* gethostbyname */
static int test_gethostbyname() 
{
    struct hostent *hp;
    char *host = "localhost";
    hp = gethostbyname(host);
    if (hp == NULL) {
        dprintf("gethostbyname failed\n");
        return -1;
    }
    print_host(hp);
    return 0;
}

/* getsockname/getpeername */
static int test_getXXXXname()
{
    struct sockaddr_un addr;
    struct sockaddr_un laddr, peeraddr;
    socklen_t addrlen, peeraddrlen;
    int sfd = 0;
    int ret = 0;

    addrlen = peeraddrlen = sizeof(struct sockaddr_un);
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);      /* Create client socket */
    if (sfd < 0) {
        dprintf("socket: %d\n", sfd);
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(SV_SOCK_PATH));
    ret = connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un));
    if (ret < 0) {
        dprintf("connect: %d\n", ret);
        return -1;
    }
    ret = getsockname(sfd, (struct sockaddr *)&laddr, &addrlen);
    if (ret < 0) {
        dprintf("getsockname ret %d\n", ret);
        return -1;
    }
    dprintf("len:%d, family:%d, sun_path:%s\n", addrlen, laddr.sun_family, laddr.sun_path);

    ret = getpeername(sfd, (struct sockaddr *)&peeraddr, &peeraddrlen); 
    if (ret < 0) {
        dprintf("getpeername ret %d", ret);
        return -1;
    }
    dprintf("len:%d, family:%d, sun_path:%s\n", peeraddrlen, peeraddr.sun_family, peeraddr.sun_path);
    shutdown(sfd, SHUT_RDWR);
    return 0;
}

#define MAXLINE 100
#define UDPPORT 8888

/* udp server */
static int test_udp_server() {
    int sfd = 0;
    ssize_t sret = 0;
    socklen_t salen, calen;
    char buf[MAXLINE];
    struct sockaddr_in saddr, caddr;
    char str[] = "hello client !";

    salen = calen = sizeof(struct sockaddr_in);
    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        dprintf("socket() error");
        return -1;
    }

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = UDPPORT;
    salen = sizeof(struct sockaddr_in);
    build_addr(&saddr.sin_addr , 127, 0, 0, 1);
    if(bind(sfd, (struct sockaddr *) &saddr, salen) < 0) {
        dprintf("bind() error");
        close(sfd);
        return -1;
    }
    sret = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *) &caddr, &calen);
    dprintf("recvfrom ret: %ld, addrlen: %d, data: %s\n", sret, calen, buf);
    if(sret < 0) {
        dprintf("recvfrom() error");
        close(sfd);
        return -1;
    }
    sret = sendto(sfd, str, sizeof(str), 0, (struct sockaddr *) &caddr, calen);
    dprintf("sendto ret: %ld, addrlen: %d, data: %s\n", sret, calen, str);
    if(sret < 0) {
        dprintf("sendto() error");
        close(sfd);
        return -1;
    }

    close(sfd);
    return 0;
}

/* udp client */
static int test_udp_client()
{
    int cfd = 0;
    ssize_t sret = 0;
    socklen_t salen = 0;
    char str[] = "hello server!";
    char buf[MAXLINE];
    struct sockaddr_in saddr;

    if((cfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        dprintf("socket() error");
        return -1;
    }
    salen = sizeof(struct sockaddr_in);
    memset(&saddr, 0, salen);
    saddr.sin_family = AF_INET;
    saddr.sin_port = UDPPORT;
    build_addr(&saddr.sin_addr , 127, 0, 0, 1);

    sret = sendto(cfd, str, sizeof(str), 0, (struct sockaddr *)&saddr, salen);
    dprintf("sendto ret: %ld, addrlen: %d, data: %s\n", sret, salen, str);
    if(sret < 0) {
        dprintf("sendto() error");
        close(cfd);
        return -1;
    }

    sret = recvfrom(cfd, buf, sizeof(buf), 0, (struct sockaddr *)&saddr, &salen);
    dprintf("recvfrom ret: %ld, addrlen: %d, data: %s\n", sret, salen, buf);
    if(sret < 0) {
        dprintf("recvfrom() error");
        close(cfd);
        return -1;
    }

    close(cfd);
    return 0;
}

#ifndef SOL_SOCKET
#define SOL_SOCKET      1
#endif

#ifndef SO_SNDBUF
#define SO_SNDBUF       7
#endif

/* getsockopt/setsockopt */
static int test_XXXsockopt()
{
    int ret = 0;
    int bufsize = 0;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    socklen_t optsize = sizeof(socklen_t);
    ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, &optsize);
    dprintf("getsockopt ret:%d errno:%d bufsize:%d optsize:%d\n",
           ret, errno, bufsize, optsize);
    if (ret < 0) {
        return -1;
    }
    bufsize = 3096;
    ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, sizeof(socklen_t));
    dprintf("setsockopt ret:%d errno:%d\n",ret, errno);
    if (ret < 0) {
        return -1;
    }
    ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, &optsize);
    dprintf("getsockopt ret:%d errno:%d bufsize:%d optsize:%d\n",
        ret, errno, bufsize, optsize);
    if (ret < 0) {
        return -1;
    }
    return 0;
}
static int test_poll()
{
    int fd1 = 0;
    int fd2 = 0;
    int ret = 0;
    char *fname1 = "/tmp/remote.file";
    char *fname2 = "/tmp/remote.file1";
    struct pollfd fds[2] = { {0}, {0} };

    dprintf("\n===test_poll start===\n");
    fd1 = open(fname1, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    fd2 = open(fname2, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    if (fd1 < 0 || fd2 < 0) {
        dprintf("\nUP>open file fail, \r\n");
        close(fd1);
        close(fd2);
        return -1;
    }
    dprintf("\nfd1:%d, fd2:%d\n", fd1, fd2);
    close(fd2);
    fds[0].fd = fd1;
    fds[0].events |= POLLIN | POLLOUT;
    fds[1].fd = fd2;
    ret = poll(fds, (nfds_t)2, 5000);
    dprintf("\nUP>poll return: %d, revents1: %d, revents2: %d\r\n",
        ret, fds[0].revents, fds[1].revents);
    close(fd1);
    return ret >= 0 ? 0 : -1;
}

static int test_poll_arg()
{
    errno = 0;
    int ret = poll(NULL, (nfds_t)2, 5000);
    CHECK_EXPECT(errno, EFAULT, errno)
    return 0;
}

static int test_select()
{
    fd_set readset, writeset, exceptset;
    struct timeval tv;
    int fd1 = 0;
    int fd2 = 0;
    int largerFd = 0;
    int ret = 0;
    char *fname1 = "/tmp/remote.file";
    char *fname2 = "/tmp/remote.file1";

    dprintf("\n===test_select start===\n");
    dprintf("===open===\n");
    fd1 = open(fname1, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    fd2 = open(fname2, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    largerFd = fd1;
    if (fd1 < fd2) {
        largerFd = fd2;
    }
    dprintf("\nfd1:%d, fd2:%d\n", fd1, fd2);
    FD_ZERO(&readset);
    FD_SET(fd1, &readset);
    FD_SET(fd2, &readset);
    FD_ZERO(&writeset);
    FD_SET(fd1, &writeset);
    FD_SET(fd2, &writeset);
    FD_ZERO(&exceptset);
    FD_SET(fd1, &exceptset);
    FD_SET(fd2, &exceptset);
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    dprintf("\nUP>select fd1: %d, fd2: %d, sizeof(fd_set):%d\r\n",
        FD_ISSET(fd1, &readset), FD_ISSET(fd2, &readset), sizeof(fd_set));
    ret = select(largerFd + 1, &readset, &writeset, &exceptset, &tv);
    dprintf("\nUP>select return: %d, sec:%ld, usec:%ld, read_fd1: %d, read_fd2: %d\r\n", 
        ret, tv.tv_sec, tv.tv_usec, FD_ISSET(fd1, &readset), FD_ISSET(fd2, &readset)); 
    dprintf("UP>select write_fd1: %d, write_fd2: %d, except_fd1: %d, except_fd2: %d\r\n",
        FD_ISSET(fd1, &writeset), FD_ISSET(fd2, &writeset), FD_ISSET(fd1, &exceptset), FD_ISSET(fd2, &exceptset));
    if (ret < 0) {
        close(fd1);
        close(fd2);
        return -1;
    }
    dprintf("\n===test_select wait test===\n");
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    struct timeval tvstandard;
    ret = gettimeofday(&tvstandard, NULL);
    if (ret != 0) {
        dprintf("get time fail, ret:%d\n", ret);
    } else {
        dprintf("get time, sec:%ld, usec:%ld\n", tvstandard.tv_sec, tvstandard.tv_usec);
    }
    dprintf("%ld seconds sleep\n", tv.tv_sec);
    ret = select(0, NULL, NULL, NULL, &tv);
    if (ret < 0) {
        dprintf("%d select fail(__nfds == 0)\n", ret);
        close(fd1);
        close(fd2);
        return -1;
    }
    ret = gettimeofday(&tvstandard, NULL);
    if (ret != 0) {
        dprintf("get time fail, ret:%d\n", ret);
    } else {
        dprintf("get time, sec:%ld, usec:%ld\n", tvstandard.tv_sec, tvstandard.tv_usec);
    }

    ret = select(-3, NULL, NULL, NULL, &tv);
    if (ret >= 0 || errno != EINVAL) {
        dprintf("\nnegative nfds, ret:%d, errno:%d = %d\n", ret, errno, EINVAL);
        close(fd1);
        close(fd2);
        return -1;
    }
    close(fd1);
    close(fd2);

    FD_SET(fd1, &readset);
    FD_SET(fd2, &readset);
    ret = select(fd2 + 1, &readset, NULL, NULL, &tv);
    if (ret >= 0 || errno != EBADF) {
        dprintf("\nclosed fd, ret:%d, errno:%d = %d\n", ret, errno, EBADF);
        return -1;
    }
    dprintf("\n===test_select end===\n");
    return 0;
}

static int test_gethostname()
{
    char name[200] = {0};
    int ret = 0;
    dprintf("\n===test_gethostname start===\n");
    ret = gethostname(name, 200);
    if (ret) {
        dprintf("gethostname fail ret: %d, errno: %d\n", ret, errno);
        return -1;
    }
    ret = gethostname(NULL, 200);
    if (ret >= 0 || errno != EFAULT) {
        dprintf("test_gethostname failed, expect errno:%d\n", EFAULT);
        return -1;
    }
    ret = gethostname(name, 0);
    if (ret >= 0 || errno != ENAMETOOLONG) {
        dprintf("test_gethostname failed, expect errno:%d\n", ENAMETOOLONG);
        return -1;
    }
    dprintf("===test_gethostname pass===\n");
    (void)name;
    return 0;
}

#ifndef SOL_SOCKET
#define SOL_SOCKET      1
#endif

#ifndef SO_DEBUG
#define SO_DEBUG        1
#define SO_SNDBUF       7
#endif

static int test_getsockopt()
{
    int ret = 0;
    int bufsize = 0;
#ifndef DEBUG_PRINTF
    (void)ret;
#endif
    dprintf("\n===test_getsockopt start===\n");
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        dprintf("UP>socket failed, ret:%d errno:%d", fd, errno);
        return -1;
    }
    socklen_t optsize = sizeof(int);
    ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, &optsize);
    if (ret) {
        dprintf("UP>getsockopt ret:%d errno:%d sendBufSize:%d optSize:%d\n",
        ret, errno, bufsize, optsize);
        (void)close(fd);
        return -1;
    }
    bufsize = 3096;
    ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, sizeof(int));
    if (ret) {
        dprintf("UP>setsockopt ret:%d errno:%d\n",ret, errno);
        (void)close(fd);
        return -1;
    }
    ret = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bufsize, &optsize);
    if (ret) {
        dprintf("UP>getsockopt ret:%d errno:%d sendBufSize:%d optSize:%d\n",
        ret, errno, bufsize, optsize);
        (void)close(fd);
        return -1;
    }
    int isDebug = 0;
    ret = getsockopt(fd, SOL_SOCKET, SO_DEBUG, (void *)&isDebug, &optsize);
    if (ret) {
        dprintf("UP>getsockopt ret:%d errno:%d isDebug:%d optSize:%d\n",
        ret, errno, isDebug, optsize);
        (void)close(fd);
        return -1;
    }
    ret = getsockopt(fd, SOL_SOCKET, SO_DEBUG, (void *)&isDebug, NULL);
    if (ret >= 0 || errno != EFAULT) {
        dprintf("test failed: NULL optsize, ret:%d errno:%d = %d\n", ret, errno, EFAULT);
        (void)close(fd);
        return -1;
    }
    ret = setsockopt(fd, SOL_SOCKET, SO_DEBUG, (void *)&isDebug, sizeof(char));
    if (ret >= 0 || errno != EINVAL) {
        dprintf("test failed: Invalid optsize, ret:%d errno:%d = %d\n", ret, errno, EINVAL);
        (void)close(fd);
        return -1;
    }
    (void)close(fd);
    ret = getsockopt(fd, SOL_SOCKET, SO_DEBUG, (void *)&isDebug, &optsize);
    if (ret >= 0 || errno != EBADF) {
        dprintf("test failed: Closed fd, ret:%d errno:%d = %d\n", ret, errno, EBADF);
        return -1;
    }
    dprintf("===test getsockopt pass===\n");
    return 0;
}

static int __test_open_err(const char *filename, int flags, unsigned mode, int eret, int eno)
{
    errno = 0;
    int ret = open(filename, flags, mode);
    CHECK_EXPECT(ret, eret, ret)
    CHECK_EXPECT(errno, eno, errno)
    return 0;
}

static int test_open_arg()
{
    int flags = REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND;
    unsigned mode = S_IRUSR | S_IWUSR;
    char buf[500];

    memset(buf, 0xff, sizeof(buf));
    buf[499] = '\0';
    if (__test_open_err(buf, flags, mode, -1, ENAMETOOLONG)) {
        return -1;
    }
    if (__test_open_err(NULL, flags, mode, -1, EFAULT)) {
        return -1;
    }
    return 0;
}

static int __test_read(int fd, void *buf, size_t len, ssize_t eret, int eno)
{
    errno = 0;
    ssize_t ret = read(fd, buf, len);
    CHECK_EXPECT(ret, (int)eret, (int)ret)
    CHECK_EXPECT(errno, eno, errno)
    return 0;
}
static int test_read_arg()
{
    char buf[10];

    if (__test_read(-1, buf, sizeof(buf), -1, EBADF)) {
        return -1;
    }
    if (__test_read(0, NULL, sizeof(buf), -1, EFAULT)) {
        return -1;
    }
    return 0;
}

static int __test_write(int fd, void *buf, size_t len, ssize_t eret, int eno)
{
    errno = 0;
    ssize_t ret = write(fd, buf, len);
    CHECK_EXPECT(ret, (int)eret, (int)ret)
    CHECK_EXPECT(errno, eno, errno)
    return 0;
}

static int test_write_arg()
{
    char buf[10];

    if (__test_write(-1, buf, sizeof(buf), -1, EBADF)) {
        return -1;
    }
    if (__test_write(1, NULL, sizeof(buf), -1, EFAULT)) {
        return -1;
    }
    return 0;
}

static int __test_close(int fd, int eret, int eno)
{
    int ret = 0;

    errno = 0;
    ret = close(fd);
    CHECK_EXPECT(ret, (int)eret, (int)ret)
    CHECK_EXPECT(errno, eno, errno)
    return 0;
}

static int test_close_arg()
{
    return __test_close(-1, -1, EBADF);
}


static int __test_unlink(const char *filename, int eret, int eno)
{
    errno = 0;
    int ret = unlink(filename);
    CHECK_EXPECT(ret, (int)eret, (int)ret)
    CHECK_EXPECT(errno, eno, errno)
    return 0;
}

static int test_unlink_arg()
{
    char buf[500];

    memset(buf, 0xff, sizeof(buf));
    buf[499] = '\0';
    if (__test_unlink(buf, -1, ENAMETOOLONG)) {
        return -1;
    }
    if (__test_unlink(NULL, -1, EFAULT)) {
        return -1;
    }
    return 0;
}

static int __test_gethostbyaddr(const void *addr, socklen_t len, int type, int eno)
{
    struct hostent *ht;

    errno = 0;
    ht = gethostbyaddr(addr, len, type);
    CHECK_EXPECT(errno, eno, errno)
    print_host(ht);
    return 0;
}

static int test_gethostbyaddr_arg()
{
    char addr[] = {127, 0, 0, 1};

    if (__test_gethostbyaddr(addr, 0, 2, EAFNOSUPPORT)) {
        return -1;
    }
    if (__test_gethostbyaddr(NULL, sizeof(addr), 2, EFAULT)) {
        return -1;
    }
    if (__test_gethostbyaddr(addr, sizeof(addr), 2, 0)) {
        return -1;
    }
    return 0;
}

static int test_dirent()
{
    int cnt = 0;
    DIR *dir = opendir("/");
    if (dir == NULL) {
        return 0;
    }
    while (1) {
        struct dirent *ent = readdir(dir);
        if (!ent) {
            break;
        }
        cnt++;
        dprintf("name:%s\n",ent->d_name);
    }
    closedir(dir);
    dprintf("total: %d\n", cnt);
    return 0;

}

static int test_FILE_fs_posix1()
{
    char *fname = "/tmp/remote1.txt";
    char *str = "A Test string being written to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fp = fopen(fname, "w+");
    if (fp == 0) {
        dprintf("\nUP>fopen file '%s' fail, ret: %lu\r\n", fname, (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>fopen file '%s' with fp = %lu\r\n", fname, (unsigned long)fp);

    size_t nmemb = strlen(str);
    size_t sz = fwrite(str, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        dprintf("\nUP>fwrite fail, sz: %u, nmemb: %u\r\n", sz, nmemb);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>fwrite to fp = %lu, size = %d, content = %s\r\n", (unsigned long)fp, sz, str);

    ret = fseeko(fp, 0, SEEK_SET);
    if (ret != 0) {
        dprintf("\nUP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    sz = fread(rbuff, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        dprintf("\nUP>fread from fp = %lu, failed, sz: %u, nmemb: %u\r\n", (unsigned long)fp, sz, nmemb);
        fclose(fp);
        return -1;
    }
    rbuff[sz] = 0;
    dprintf("\nUP>fread from fp = %lu, size = %d, "
    "printing contents below .. %s\r\n", (unsigned long)fp, sz, rbuff);

    remove(fname);
    return ret >= 0 ? 0 : -1;
}

static int test_FILE_fs_posix2()
{
    char *str = "A Test string being written to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a tmpfile on host and writing to it..\r\n");
    fp = tmpfile();
    if (fp == 0) {
        dprintf("\nUP>tmpfile fail, ret: %lu\r\n", (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>tmpfile with fp = %lu\r\n", (unsigned long)fp);

    size_t nmemb = strlen(str);
    size_t sz = fwrite(str, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        dprintf("\nUP>fwrite fail, sz: %u, nmemb: %u\r\n", sz, nmemb);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>fwrite to fp = %lu, size = %d, content = %s\r\n", (unsigned long)fp, sz, str);

    ret = fseeko(fp, 0, SEEK_SET);
    if (ret != 0) {
        dprintf("\nUP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    sz = fread(rbuff, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        dprintf("\nUP>fread from fp = %lu, failed, sz: %u, nmemb: %u\r\n", (unsigned long)fp, sz, nmemb);
        fclose(fp);
        return -1;
    }
    rbuff[sz] = 0;
    dprintf("\nUP>fread from fp = %lu, size = %d, "
    "printing contents below .. %s\r\n", (unsigned long)fp, sz, rbuff);

    return ret >= 0 ? 0 : -1;
}

static int test_FILE_fs_posix3()
{
    char *fname = "/tmp/remote3.txt";
    char *fnameNew = "/tmp/remoteNew3.txt";
    char *strAdd = "A Test string being Add to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fp = fopen(fname, "w+");
    if (fp == 0) {
        dprintf("\nUP>fopen file '%s' fail, ret: %lu\r\n", fname, (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>fopened file '%s' with fp = %lu\r\n", fname, (unsigned long)fp);

    rename(fname, fnameNew);
    dprintf("\nUP>rename form '%s' to '%s'\r\n", fname, fnameNew);

    fp = freopen(fnameNew, "a", fp);
    if (fp == 0) {
        dprintf("\nUP>freopen file '%s' fail, ret: %lu\r\n", fnameNew, (unsigned long)fp);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>freopened file '%s' with fp = %lu\r\n", fnameNew, (unsigned long)fp);

    ret = fprintf(fp, strAdd);
    if (ret < 0) {
        dprintf("\nUP>fprintf fail.\n");
        fclose(fp);
        return -1;
    }

    dprintf("\nUP>test stdin stdout stderr\n");
    ret = fprintf(stdin, "test stdin\n");
    if (ret < 0) {
        dprintf("\nUP>fprintf fail.\n");
        fclose(fp);
        return -1;
    }

    ret = fprintf(stdout, "test stdout\n");
    if (ret < 0) {
        dprintf("\nUP>fprintf fail.\n");
        fclose(fp);
        return -1;
    }

    ret = fprintf(stderr, "test stderr\n");
    if (ret < 0) {
        dprintf("\nUP>fprintf fail.\n");
        fclose(fp);
        return -1;
    }

    ret = fseeko(fp, 0, SEEK_SET);
    if (ret != 0) {
        dprintf("\nUP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    size_t nmemb = strlen(strAdd);
    size_t sz = fread(rbuff, sizeof(char), nmemb, fp);
    if (sz != 0) {
        dprintf("\nUP>file is add only, should not read.\r\n");
        fclose(fp);
        return -1;
    }

    fp = freopen(fnameNew, "r", fp);
    sz = fread(rbuff, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        dprintf("\nUP>fread from fp = %lu, failed, sz: %u, nmemb: %u\r\n", (unsigned long)fp, sz, nmemb);
        fclose(fp);
        return -1;
    }
    rbuff[sz] = 0;
    dprintf("\nUP>fread from fp = %lu, size = %d, "
    "printing contents below .. %s\r\n", (unsigned long)fp, sz, rbuff);

    ret = fclose(fp);
    dprintf("\nUP>fclosed fp = %lu\r\n", (unsigned long)fp);
    remove(fnameNew);
    return ret >= 0 ? 0 : -1;
}

static int test_FILE_fs_posix4()
{
    char *fname = "/tmp/remote4.txt";
    char *str = "A Test string being written to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fp = fopen(fname, "w");
    if (fp == 0) {
        dprintf("\nUP>fopen file '%s' fail, ret: %lu\r\n", fname, (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>fopen file '%s' with fp = %lu\r\n", fname, (unsigned long)fp);

    ret = fprintf(fp, str);
    if (ret < 0) {
        dprintf("\nUP>fprintf fail.\n");
        fclose(fp);
        return -1;
    }

    size_t nmemb = strlen(str);
    size_t sz = fread(rbuff, sizeof(char), nmemb, fp);
    if (ferror(fp) == 0) { // 只写模式，不应该读成功
        dprintf("\nUP>file is write only, should not read.\n");
        fclose(fp);
        return -1;
    }

    clearerr(fp);
    if (ferror(fp) != 0) {
        dprintf("\nUP>clearerr fail.\n");
        fclose(fp);
        return -1;
    }

    ret = fclose(fp);
    dprintf("\nUP>fclosed fp = %lu\r\n", (unsigned long)fp);
    remove(fname);
    return ret >= 0 ? 0 : -1;
}

static int test_FILE_fs_posix5()
{
    char *fname = "/tmp/remote5.txt";
    char *str = "This is a Test string being written to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fp = fopen(fname, "w+");
    if (fp == 0) {
        dprintf("\nUP>fopen file '%s' fail, ret: %lu\r\n", fname, (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>fopened file '%s' with fp = %lu\r\n", fname, (unsigned long)fp);

    ret = fputs(str, fp);
    if (ret < 0) {
        dprintf("\nUP>fputs fail, ret: %d\r\n", ret);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>fputs to fp = %lu, content = %s\r\n", (unsigned long)fp, str);

    long pos = ftello(fp);
    size_t slen = strlen(str);
    size_t realPos = slen;
    if (pos != realPos) {
        dprintf("\nUP>ftello get pos fail, pos: %d, realPos: %d\r\n", pos, realPos);
        fclose(fp);
        return -1;
    }

    ret = fseeko(fp, 0, SEEK_SET);
    if (ret != 0) {
        dprintf("\nUP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    pos = ftello(fp);
    realPos = 0;
    if (pos != realPos) {
        dprintf("\nUP>reset pos fail, pos: %d, realPos: %d\r\n", pos, realPos);
        fclose(fp);
        return -1;
    }

    int isEof = feof(fp);
    if (isEof) {
        dprintf("\nUP>should not EOF\r\n");
        fclose(fp);
        return -1;
    }

    int ch = getc(fp);
    if (ch != *str) {
        dprintf("\nUP>getc fail, ch: %c\r\n", ch);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>getc, fp = %lu, ch = %c\r\n", (unsigned long)fp, ch);

    ch = getc_unlocked(fp);
    if (ch != *(str + 1)) {
        dprintf("\nUP>getc_unlocked fail, ch: %c\r\n", ch);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>getc_unlocked, fp = %lu, ch = %c\r\n", (unsigned long)fp, ch);

    remove(fname);
    return ret >= 0 ? 0 : -1;
}


static int test_FILE_fs_posix6()
{
    char *fname = "/tmp/remote6.txt";
    char *str = "A Test string being written to file..";
    FILE *fp;
    int ret = 0;
    char rbuff[100];
    struct stat statbuff = {0}; 

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fp = fopen(fname, "w+");
    if (fp == 0) {
        dprintf("\nUP>fopen file '%s' fail, ret: %lu\r\n", fname, (unsigned long)fp);
        return -1;
    }
    dprintf("\nUP>fopened file '%s' with fp = %lu\r\n", fname, (unsigned long)fp);

    ret = stat(fname, &statbuff);
    if (ret != 0) {
        dprintf("\nUP>stat fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>get file stat, size:%ld, access time:%ld\r\n",statbuff.st_blksize, statbuff.st_atim.tv_sec);

    size_t slen = strlen(str);
    ret = fputs(str, fp);
    if (ret < 0) {
        dprintf("\nUP>fputs fail, ret: %d\r\n", ret);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>fputs to fp = %lu, content = %s\r\n", (unsigned long)fp, str);

    ret = fflush(fp);
    if (ret != 0) {
        dprintf("\nUP>fflush fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    ret = stat(fname, &statbuff);
    if (ret != 0) {
        dprintf("\nUP>stat fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>get file stat, size:%ld, access time:%ld\r\n",statbuff.st_blksize, statbuff.st_atim.tv_sec);

    ret = fseeko(fp, 0, SEEK_SET);
    if (ret != 0) {
        dprintf("\nUP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    char *retStr = fgets(rbuff, slen + 1, fp);
    if (retStr == NULL) {
        dprintf("\nUP>fgets fail. \r\n");
        fclose(fp);
        return -1;
    }
    dprintf("\nUP>fgets fp = %lu, size = %d, "
    "printing contents below .. %s\r\n", (unsigned long)fp, slen, rbuff);

    retStr = fgets(rbuff, slen + 1, fp); // 再次获取一遍
    if (retStr != NULL) {
        dprintf("\nUP>fgets should get nothing, str: %s.\r\n", rbuff);
        fclose(fp);
        return -1;
    }

    int isEof = feof(fp);
    if (!isEof) {
        dprintf("\nUP>should EOF\r\n");
        fclose(fp);
        return -1;
    }

    ret = fclose(fp);
    dprintf("\nUP>fclosed fp = %lu\r\n", (unsigned long)fp);
    remove(fname);
    return ret >= 0 ? 0 : -1;
}

static int test_FILE_fs_posix7()
{
    FILE *fp;
    char buffer[100] = {0};
    struct stat statbuff = {0}; 

    fp = popen("pwd", "r");
    if (fp == 0) {
        dprintf("popen failed!");
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        dprintf("%s", buffer);
    }

    int ret = stat("./none123321.txt", &statbuff);
    if (ret != -1 || errno != ENOENT) {
        dprintf("\nUP>stat should ENOENT, but get ret:%p, errstr:%s\r\n", ret, strerror(errno));
        return -1;
    }

    ret = pclose(fp);
    return (ret == 0 ? 0 : -1);
}

static int test_getcwd()
{
    char buff_short[1] = {0};
    char buff_norm[200] = {0};
    char *buf = getcwd(NULL, 0);
    if (buf == NULL) {
        dprintf("\nUP>getcwd null fail, %s\r\n", strerror(errno));
        return -1;
    }
    dprintf("\nUP>getcwd null buff: %s\r\n", buf);

    buf = getcwd(buff_norm, sizeof(buff_norm));
    if (buf == NULL) {
        dprintf("\nUP>getcwd norm fail, %s\r\n", strerror(errno));
        return -1;
    }
    if (buf != &(buff_norm[0])) {
        dprintf("\nUP>getcwd norm fail pointer not same\r\n");
        return -1;
    }
    dprintf("\nUP>getcwd norm buff: %s\r\n", buff_norm);

    buf = getcwd(buff_short, sizeof(buff_short));
    if (buf != NULL || errno != ERANGE) {
        dprintf("\nUP>getcwd short should ERANGE, but get buf:%p, errstr:%s\r\n", buf, strerror(errno));
        return -1;
    }

    buf = getcwd(buff_norm, 0);
    if (buf != NULL || errno != EINVAL) {
        dprintf("\nUP>getcwd(NULL, 100) should EINVAL, but get buf:%p, errstr:%s\r\n", buf, strerror(errno));
        return -1;
    }
    return 0;
}

static int test_FILE_fs_posix8()
{
    char *fname = "/tmp/remote.file8";
    char *str = "A Test string being written to file..";
    int fd = 0;
    int ret = 0;
    off_t off = 0;
    char rbuff[100];
    struct stat statbuff = {0};

    dprintf("\nUP>Creating a file on host and writing to it..\r\n");
    fd = open(fname, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    if (fd < 0) {
        dprintf("\nUP>Open file '%s' fail, ret: %d\r\n", fname, fd);
        return fd;
    }
    dprintf("\nUP>Opened file '%s' with fd = %d\r\n", fname, fd);

    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        dprintf("\nUP>write fail, ret: %d\r\n", ret);
        goto close_file;
    }

    FILE *f = fdopen(fd, "r+");
    if (f == NULL) {
        dprintf("\nUP>fdopen '%s' fail, ret: %d\r\n", fname, fd);
        ret = -1;
        goto close_file;
    }

    ret = fstat(fd, &statbuff);
    if (ret < 0) {
        dprintf("\nUP>fstat failed, but get ret:%p, errstr:%s\r\n", ret, strerror(errno));
        goto close_file;
    }

    ret = fileno(f);
    if (ret < 0 || ret != fd) {
        dprintf("\nUP>fileno failed, ret:%d, errstr:%s\r\n", ret, strerror(errno));
        ret = -1;
        goto close_file;
    }

close_file:
    close(fd);
    dprintf("\nUP>Closed fd = %d\r\n", fd);
    return ret >= 0 ? 0 : -1;
}

static int test_link()
{
    char buf[20];
    #define SLNK "/tmp/test"
    #define SLNK_TARGET "/tmp/nonexist"
    int ret = system("ln -sf /tmp/nonexist /tmp/test");
    if (ret) {
        printf("system error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }
    ssize_t sret = readlink("/tmp/test", buf, sizeof(buf));
    if(sret < 0 || strncmp(SLNK_TARGET, buf, sizeof(SLNK_TARGET))) {
        printf("readlink error ret: %ld, errstr: %s\n", sret, strerror(errno));
        return -1;
    }
    unlink(SLNK);
    return 0;
}

static int test_access()
{
    char *fname = "/tmp/test_access.file";
    char *fname2 = "/tmp/non-exist";
    char *str = "A Test string being written to file..";
    int fd = open(fname, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
        S_IRUSR | S_IWUSR);

    int ret = access(fname, F_OK|R_OK|W_OK);
    if (ret) {
        printf("UP>access error ret: %d, errstr: %s\n", ret, strerror(errno));
        close(fd);
        return -1;
    }

    ret = access(fname2, F_OK);
    printf("UP>acces ret: %d, errstr: %s\n", ret, strerror(errno));
    if (ret != -1) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int test_dup2()
{
    char *fname = "/tmp/test_dup2.file";
    char *fname2 = "/tmp/test_dup2_tmp.file";
    char *str = "A Test string being written to file..";
    char rbuff[100];
    int fd = open(fname, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
        S_IRUSR | S_IWUSR);
    int fd2 = open(fname2, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
        S_IRUSR | S_IWUSR);

    int ret = dup2(fd, fd2);
    if (ret != fd2) {
        printf("UP>dup2 error ret: %d, fd2:%d errstr: %s\n", ret, fd2, strerror(errno));
        ret = -1;
        goto close_file;
    }

    ret = write(fd2, str, strlen(str));

    off_t off = lseek(fd, 0, SEEK_SET);
    if (off < 0) {
        dprintf("\nUP>lseek fail, ret %d\r\n", ret);
        goto close_file;
    }
    ret = read(fd, rbuff, sizeof(rbuff));
    if (ret < 0) {
        dprintf("\nUP>Read from fd = %d, failed ret %d\r\n", fd, ret);
        goto close_file;
    }
    rbuff[ret] = 0;
    dprintf("\nUP>Read from fd = %d, size = %d, "
    "printing contents below .. %s\r\n", fd, ret, rbuff);

close_file:
    close(fd);
    close(fd2);
    dprintf("\nUP>Closed fd = %d, %d\r\n", fd, fd2);
    return ret >= 0 ? 0 : -1;
}

// 测试mkfifo必须配合linux侧的其他进程的读写操作
static int test_mkfifo()
{
    char *fname = "/tmp/test_mkfifo";
    char rbuff[100];
    pthread_t mkfifo_thread;
    mkfifo(fname, 0666);
    
    // test read
    dprintf("\nUP>mkfifo reading, please run 'echo \"any test sting\" > /tmp/test_mkfifo'\r\n");
    int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        dprintf("\nUP>test_mkfifo open file '%s' fail, ret: %d\r\n", fname, fd);
        return fd;
    }
    dprintf("\nUP>test_mkfifo opened file '%s' with fd = %d\r\n", fname, fd);

    ssize_t ret = read(fd, rbuff, sizeof(rbuff));
    if (ret < 0) {
        dprintf("\nUP>Read from fd = %d, failed ret %d\r\n", fd, ret);
        goto close_file;
    }
    rbuff[ret] = 0;
    dprintf("\nUP>Read from fd = %d, size = %d, "
    "printing contents below .. %s\r\n", fd, ret, rbuff);
    close(fd);

    // test write
    char *str = "A Test string being written to file..";
    dprintf("\nUP>mkfifo writing, please run 'cat /tmp/test_mkfifo'\r\n");
    fd = open(fname, O_WRONLY);
    if (fd < 0) {
        dprintf("\nUP>test_mkfifo open file '%s' fail, ret: %d\r\n", fname, fd);
        ret = -1;
        goto close_file;
    }
    dprintf("\nUP> test_mkfifo opened file '%s' with fd = %d\r\n", fname, fd);

    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        dprintf("\nUP>test_mkfifo write to fd = %d, failed ret %d\r\n", fd, ret);
         ret = -1;
        goto close_file;
    }

close_file:
    close(fd);
    dprintf("\nUP>Closed fd = %d\r\n", fd);
    return ret >= 0 ? 0 : -1;
}

static int test_dir_mk_ch_rm()
{
    char *root_dir = "/tmp";
    char *dir1 = "/tmp/diraa1";
    char *dir2 = "/tmp/diraa1/diraa2";
    char *dir3 = "./diraa3";
    int ret;

    ret = chdir(root_dir);
    if (ret) {
        printf("UP>chdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = mkdir(dir1, 0744);
    if (ret) {
        printf("UP>mkdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = mkdir(dir2, 0744);
    if (ret) {
        printf("UP>mkdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = mkdir(dir3, 0744);
    if (ret) {
        printf("UP>mkdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = chmod(dir3, 0755);
    if (ret) {
        printf("UP>chmod error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = rmdir(dir2);
    if (ret) {
        printf("UP>rmdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = rmdir(dir1);
    if (ret) {
        printf("UP>rmdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    ret = rmdir(dir3);
    if (ret) {
        printf("UP>rmdir error ret: %d, errstr: %s\n", ret, strerror(errno));
        return -1;
    }

    return 0;
}

static int test_fseek_ftell()
{
    char *fname = "/tmp/test_fseek_ftell.txt";
    FILE *fp;
    char buff[100];
    char *wdata = "hello world!\nThis is a demo..\n";
    int ret;

    fp = fopen(fname, "w+");
    if (fp == NULL) {
        printf("UP>fopen file '%s' fail, ret: %lu\n", fname, (unsigned long)fp);
        return -1;
    }
    printf("UP>fopen file '%s' success\n", fname);

    size_t nmemb = strlen(wdata);
    size_t sz = fwrite(wdata, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        printf("UP>fwrite fail, sz: %u, nmemb: %u\r\n", sz, nmemb);
        fclose(fp);
        return -1;
    }

    ret = fseek(fp, 0, SEEK_SET);
    if (ret != 0) {
        printf("UP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    ret = fread(buff, sizeof(char), sizeof(buff) - 1, fp);
    if (ret != nmemb) {
        printf("UP>fread fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }
    buff[99] = '\0';
    printf("UP>read:%s\n", buff);

    ret = fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    printf("UP>filesize:%ld\n", size);

    fclose(fp);
    return 0;
}

static int test_pipe()
{
    int fd[2];
    char buff[128];
    int flags;

    if (pipe(fd) == -1) {
        printf("UP>pipe fail\n");
        return -1;
    }
    printf("UP>pipe fd[2]: %d %d\n", fd[0], fd[1]);

    close(fd[0]);
    close(fd[1]);

    uint16_t host_d = 0x1055;
    uint16_t net_d = htons(host_d);
    printf("UP>htons(%04x) = %04x\n", host_d, net_d);

    net_d = 0x1055;
    host_d = ntohs(net_d);
    printf("UP>ntohs(%04x) = %04x\n", net_d, host_d);
    return 0;
}

static int test_fscanfx()
{
    char *fname = "/tmp/test_fscanx.txt";
    FILE *fp;
    char buff[100];
    char *wdata = "0x123456789ABCDEF0 0xABCDEF0123456789\n0x123456789ABCDEF0 0xABCDEF0123456789\n";
    int ret;
    uint64_t data;
    int i;

    fp = fopen(fname, "w+");
    if (fp == NULL) {
        printf("UP>fopen file '%s' fail, ret: %lu\n", fname, (unsigned long)fp);
        return -1;
    }
    printf("UP>fopen file '%s' success\n", fname);

    size_t nmemb = strlen(wdata);
    size_t sz = fwrite(wdata, sizeof(char), nmemb, fp);
    if (sz != nmemb) {
        printf("UP>fwrite fail, sz: %u, nmemb: %u\r\n", sz, nmemb);
        fclose(fp);
        return -1;
    }

    ret = fseek(fp, 0, SEEK_SET);
    if (ret != 0) {
        printf("UP>fseeko fail, ret %d\r\n", ret);
        fclose(fp);
        return -1;
    }

    for (i = 0; i < 5; i++) {
        ret = fscanf(fp, "%llx ", &data);
        printf("fscanf ret:%d data:0x%llx\r\n", ret, data);
        if (ret <= 0) {
            break;
        }
    }

    fclose(fp);
    return 0;
}

static int test_ifnameindex()
{
    struct if_nameindex *if_ni, *i;
    if_ni = if_nameindex();
    dprintf("test_ifnameindex, 0x%x\n",if_ni);

    if (if_ni == NULL) {
        dprintf("if_nameindex");
        return -1;
    }
    for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++)
        dprintf("%u: %s\n", i->if_index, i->if_name);
    if_freenameindex(if_ni);
    return 0;
}

static int test_putchar()
{
    putchar('a');
    putchar('b');
    putchar('c');
    putchar('1');
    putchar('2');
    putchar('3');

    return 0;
}

static int test_gaistrerror()
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    int ret = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = 20; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    ret = getaddrinfo(NULL, "22", &hints, &result);
    dprintf("test_gaistrerror, ret %d\n", ret);
    if (ret < 0) {
        dprintf("getaddrinfo: %s\n", gai_strerror(ret));
        return 0;
    }
    dprintfaddrinfo(result);
    freeaddrinfo(result);
    return -1;
}

static int test_accept4()
{
    return 0;
}

static int test_writev()
{
    char *str0 = "hello ";
    char *str1 = "world\n";
    struct iovec iov[2];
    ssize_t nwritten;

    iov[0].iov_base = str0;
    iov[0].iov_len = strlen(str0) + 1;
    iov[1].iov_base = str1;
    iov[1].iov_len = strlen(str1) + 1;

    nwritten = writev(STDOUT_FILENO, iov, 2);
    printf("%ld bytes written.\n", nwritten);
    
    return 0;
}

typedef int (*test_fn)();
typedef struct test_case {
    char *name;
    test_fn fn;
    int skip;
} test_case_t;

#define TEST_CASE(func, s) {         \
    .name = #func,                   \
    .fn = func,                      \
    .skip = s                        \
}

#define TEST_CASE_Y(func) TEST_CASE(func, 0)
#define TEST_CASE_N(func) TEST_CASE(func, 1)

static test_case_t g_cases[] = {
    TEST_CASE_Y(test_fs_posix1),
    TEST_CASE_Y(test_fs_posix2),
    TEST_CASE_Y(test_fs_posix3),
    TEST_CASE_N(test_domain_server),
    TEST_CASE_N(test_domain_client),
    TEST_CASE_Y(test_getaddrinfo),
    TEST_CASE_Y(test_gethostbyaddr),
    TEST_CASE_Y(test_gethostbyname),
    TEST_CASE_N(test_getXXXXname),
    TEST_CASE_N(test_udp_server),
    TEST_CASE_N(test_udp_client),
    TEST_CASE_N(test_XXXsockopt),
    TEST_CASE_Y(test_poll),
    TEST_CASE_Y(test_poll_arg),
    TEST_CASE_Y(test_select),
    TEST_CASE_N(test_gethostname),
    TEST_CASE_N(test_getsockopt),
    TEST_CASE_Y(test_open_arg),
    TEST_CASE_Y(test_read_arg),
    TEST_CASE_Y(test_write_arg),
    TEST_CASE_Y(test_close_arg),
    TEST_CASE_Y(test_gethostbyaddr_arg),
    TEST_CASE_Y(test_dirent),
    TEST_CASE_Y(test_FILE_fs_posix1),
    TEST_CASE_Y(test_FILE_fs_posix2),
    TEST_CASE_Y(test_FILE_fs_posix3),
    TEST_CASE_Y(test_FILE_fs_posix4),
    TEST_CASE_Y(test_FILE_fs_posix5),
    TEST_CASE_Y(test_FILE_fs_posix6),
    TEST_CASE_Y(test_FILE_fs_posix7),
    TEST_CASE_Y(test_FILE_fs_posix8),
    TEST_CASE_Y(test_getcwd),
    TEST_CASE_Y(test_link),
    TEST_CASE_Y(test_access),
    TEST_CASE_Y(test_dup2),
    TEST_CASE_N(test_mkfifo),
    TEST_CASE_Y(test_dir_mk_ch_rm),
    TEST_CASE_Y(test_fseek_ftell),
    TEST_CASE_Y(test_pipe),
    TEST_CASE_N(test_fscanfx),
    TEST_CASE_Y(test_ifnameindex),
    TEST_CASE_Y(test_putchar),
    TEST_CASE_Y(test_gaistrerror),
    TEST_CASE_N(test_accept4),
    TEST_CASE_Y(test_writev),
};

int rpc_test_entry()
{
    int i = 0;
    int cnt = 0;
    int fails = 0;
    int len = sizeof(g_cases) / sizeof(test_case_t);
    dprintf("\n===test start=== \n");
    for (i = 0, cnt = 0; i < len; i++) {
        test_case_t *tc = &g_cases[i];
        dprintf("\n===%s start === \n", tc->name);
        if (tc->skip) {
            continue;
        }
        cnt++;
        int ret = tc->fn();
        if (ret) {
            fails++;
            dprintf("\n===%s failed ===\n", tc->name);
        } else {
            dprintf("\n===%s success ===\n", tc->name);
        }
    }
    dprintf("\n===test end, total: %d, fails:%d ===\n", cnt, fails);
    return cnt;
}
