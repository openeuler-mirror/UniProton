#ifndef _RPC_ERR_H
#define _RPC_ERR_H

#define RPC_EBASE        3000
#define RPC_ENO_SLOT     (RPC_EBASE + 1)
#define RPC_EOVERLONG    (RPC_EBASE + 2)
#define RPC_ECORRUPTED   (RPC_EBASE + 3)
#define RPC_ENEED_INIT   (RPC_EBASE + 4)
#define RPC_EINVAL       (RPC_EBASE + 5)
#define RPC_ENOMEM       (RPC_EBASE + 6)

#define SOCKER_EBASE     4000
#define SOCKET_GETFD     (SOCKER_EBASE + 1)
#define SOCKET_FIND      (SOCKER_EBASE + 2)
#define SOCKET_LOCAL     (SOCKER_EBASE + 3)
#define SOCKET_PROXY     (SOCKER_EBASE + 4)
#define SOCKET_NOMEM     (SOCKER_EBASE + 5)

#endif /* _RPC_ERR_H */