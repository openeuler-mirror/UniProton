#include <stdlib.h>
#include <pthread.h>

void *abort_1_1_a_thread_func()
{
    printf("entry abort_1_1_a_thread_func\n");
    while(1) {
        printf("runing abort_1_1_a_thread_func\n");
        sleep(1);
    }
    return NULL;
}

void *abort_1_1_b_thread_func()
{
    printf("entry abort_1_1_b_thread_func\n");    
    while(1) {
        printf("runing abort_1_1_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

int abort_1_1()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, abort_1_1_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, abort_1_1_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    printf("abort_1_1 sleep 2s\n");
    sleep(2);
    abort();
    printf("abort_1_1 can't run here\n");
    return 0;
}