#ifndef PRT_POSIX_INTERNAL_H
#define PRT_POSIX_INTERNAL_H

#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_task_external.h"
#include "prt_list_external.h"
#include "errno.h"

#define PRT_SCHED_FIFO          1
#define PTHREAD_DEFAULT_POLICY  PRT_SCHED_FIFO

#define PTHREAD_DEFAULT_PRIORITY 10
#define PTHREAD_OP_FAIL          (-1)

#define PTHREAD_ATTR_UNINIT 0
#define PTHREAD_ATTR_INIT   1

typedef void * (*prt_pthread_startroutine)(void *);

extern int PRT_PthreadCreate(TskHandle *thread, const pthread_attr_t *attrp,
                             prt_pthread_startroutine routine, void *arg);
extern void PRT_PthreadExit(void *retval);
extern int PRT_PthreadKeyCreate(pthread_key_t *key, void (*destructor)(void *));
extern int PRT_PthreadKeyDelete(pthread_key_t key);
extern int PRT_PthreadSetSpecific(pthread_key_t key, const void *value);
extern void *PRT_PthreadGetSpecific(pthread_key_t key);
extern int PRT_PthreadJoin(TskHandle thread, void **status);
extern int PRT_PthreadDetach(TskHandle thread);
extern void PRT_PthreadTestCancel(void);
extern int PRT_PthreadCancel(TskHandle thread);
extern int PRT_PthreadSetCancelState(int state, int *oldstate);
extern int PRT_PthreadSetCancelType(int type, int *oldType);
extern U32 OsTimeSpec2Tick(const struct timespec *tp);
extern U32 OsTimeOut2Ticks(const struct timespec *time, U32 *ticks);

#endif

