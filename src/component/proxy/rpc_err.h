#ifndef _RPC_ERR_H
#define _RPC_ERR_H

#define RPC_EBASE        3000
#define RPC_ENO_SLOT     (RPC_EBASE + 1)
#define RPC_EOVERLONG    (RPC_EBASE + 2)
#define RPC_ECORRUPTED   (RPC_EBASE + 3)
#define RPC_ENEED_INIT   (RPC_EBASE + 4)
#define RPC_EINVAL       (RPC_EBASE + 5)
#define RPC_ENOMEM       (RPC_EBASE + 6)

#endif /* _RPC_ERR_H */