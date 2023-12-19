#ifndef _PRINT_H_
#define _PRINT_H_

#define RTOS_STOP 1U

typedef struct {
    unsigned long long size;
    unsigned long long head;
    unsigned long long tail;
    unsigned long long stop;
} PrintHead;

extern PrintHead *g_printHead;

#endif