#ifndef _PRT_NOTIFIER_H_
#define _PRT_NOTIFIER_H_

#define NOTIFY_DONE     0x0000      /* Don't care */
#define NOTIFY_OK       0x0001      /* Suits me */
#define NOTIFY_STOP_MASK    0x8000      /* Don't call further */
#define NOTIFY_BAD      (NOTIFY_STOP_MASK|0x0002)
                        /* Bad/Veto action */
/*
 * Clean way to return from the notifier and stop further calls.
 */
#define NOTIFY_STOP     (NOTIFY_OK|NOTIFY_STOP_MASK)

struct NotifierBlock;

typedef int (*NotifierFn)(struct NotifierBlock *nb,
            int action, void *data);

struct NotifierBlock {
    NotifierFn call;
    struct NotifierBlock *next;
    int priority;
};

struct NotifierHead {
    struct NotifierBlock *head;
};

extern int OsNotifierChainRegister(struct NotifierHead *nh,
        struct NotifierBlock *n);

extern int OsNotifierChainUnRegister(struct NotifierHead *nh,
        struct NotifierBlock *n);

extern int OsNotifierCallChain(struct NotifierHead *nh,
                   int val, void *v,
                   int nr_to_call, int *nr_calls);

extern int OsNotifierRawCallChain(struct NotifierHead *nh,
                   int val, void *v,
                   int nr_to_call, int *nr_calls);

int OsNotifyDie(int val, void *v);

int OsRegisterDieNotifier(struct NotifierBlock *nb);

int OsUnregisterDieNotifier(struct NotifierBlock *nb);
#endif /* _PRT_NOTIFIER_H_ */