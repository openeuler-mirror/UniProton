#ifndef _RPC_INTERNAL_COMMON_H
#define _RPC_INTERNAL_COMMON_H

#define FD_SETSIZE 1024
#define UNUSED(arg) ((void)arg)

#define STATUS_READY       0
#define STATUS_WAITING     1

#ifndef NO_STD_HEADERS
#include <wctype.h>
#include <bits/alltypes.h>
#include <poll.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#else 
typedef unsigned int uint32_t;

//  bits/sockaddr.h
typedef unsigned short int sa_family_t;
//  bits/socket.h

struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};


//  bits/types.h

typedef unsigned int __socklen_t;

//  unistd.h
typedef __socklen_t socklen_t;

//  netdb.h

/* Structure to contain information about address of a service provider.  */
struct addrinfo {
    int ai_flags;                 /* Input flags.  */
    int ai_family;                /* Protocol family for socket.  */
    int ai_socktype;              /* Socket type.  */
    int ai_protocol;              /* Protocol for socket.  */
    socklen_t ai_addrlen;         /* Length of socket address.  */
    struct sockaddr *ai_addr;     /* Socket address for socket.  */
    char *ai_canonname;           /* Canonical name for service location.  */
    struct addrinfo *ai_next;     /* Pointer to next in list.  */
};


/* Description of data base entry for a single host.  */
struct hostent {
    char *h_name;                 /* Official name of host.  */
    char **h_aliases;             /* Alias list.  */
    int h_addrtype;               /* Host address type.  */
    int h_length;                 /* Length of address.  */
    char **h_addr_list;           /* List of addresses from name server.  */
};

//  bits/socket_type.h
/* Types of sockets.  */
enum __socket_type {
    SOCK_STREAM = 1,              /* Sequenced, reliable, connection-based
                                   byte streams.  */
#define SOCK_STREAM SOCK_STREAM
    SOCK_DGRAM = 2,               /* Connectionless, unreliable datagrams
                                   of fixed maximum length.  */
#define SOCK_DGRAM SOCK_DGRAM
    SOCK_RAW = 3,                 /* Raw protocol interface.  */
#define SOCK_RAW SOCK_RAW
    SOCK_RDM = 4,                 /* Reliably-delivered messages.  */
#define SOCK_RDM SOCK_RDM
    SOCK_SEQPACKET = 5,           /* Sequenced, reliable, connection-based,
                                   datagrams of fixed maximum length.  */
#define SOCK_SEQPACKET SOCK_SEQPACKET
    SOCK_DCCP = 6,                /* Datagram Congestion Control Protocol.  */
#define SOCK_DCCP SOCK_DCCP
    SOCK_PACKET = 10,             /* Linux specific way of getting packets
                                   at the dev level.  For writing rarp and
                                   other similar things on the user level. */
#define SOCK_PACKET SOCK_PACKET

  /* Flags to be ORed into the type parameter of socket and socketpair and
     used for the flags parameter of paccept.  */

    SOCK_CLOEXEC = 02000000,      /* Atomically set close-on-exec flag for the
                                   new descriptor(s).  */
#define SOCK_CLOEXEC SOCK_CLOEXEC
    SOCK_NONBLOCK = 00004000      /* Atomically mark descriptor(s) as
                                   non-blocking.  */
#define SOCK_NONBLOCK SOCK_NONBLOCK
};


//  sys/poll.h

#define POLLIN     0x001
#define POLLPRI    0x002
#define POLLOUT    0x004
#define POLLERR    0x008
#define POLLHUP    0x010
#define POLLNVAL   0x020
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#ifndef POLLWRNORM
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200
#endif
#ifndef POLLMSG
#define POLLMSG    0x400
#define POLLRDHUP  0x2000
#endif

typedef unsigned long int nfds_t;

/* Data structure describing a polling request.  */
struct pollfd {
    int fd;                     /* File descriptor to poll.  */
    short int events;           /* Types of events poller cares about.  */
    short int revents;          /* Types of events that actually occurred.  */
};

//  sys/un.h

struct sockaddr_un {
    sa_family_t sun_family;
    char sun_path[108];
};

//  sys/socket.h
enum {
    SHUT_RD = 0,          /* No more receptions.  */
#define SHUT_RD         SHUT_RD
    SHUT_WR,              /* No more transmissions.  */
#define SHUT_WR         SHUT_WR
    SHUT_RDWR             /* No more receptions or transmissions.  */
#define SHUT_RDWR       SHUT_RDWR
};

//  netinet/in.h

typedef uint32_t in_addr_t;
struct in_addr {
    in_addr_t s_addr;
};

typedef uint16_t in_port_t;
struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr)
      - (sizeof (unsigned short int))
      - sizeof (in_port_t)
      - sizeof (struct in_addr)];
};
#endif /* NO_STD_HEADERS */

typedef uintptr_t fileHandle;

void OsProxyFreeAddrList(struct addrinfo *ai);
int OsProxyEncodeAddrList(const struct addrinfo *ai, char *buf, int *buflen);
int OsProxyDecodeAddrList(const char *buf, int cnt, int buflen, struct addrinfo **out);
int OsProxyDecodeHostEnt(struct hostent **ppht, char *src_buf, int buflen);
int OsProxyEncodeHostEnt(struct hostent *ht, char *buf, int buflen);

#endif /* _RPC_INTERNAL_COMMON_H */
