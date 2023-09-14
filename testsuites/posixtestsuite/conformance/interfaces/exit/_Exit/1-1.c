#include <stdlib.h>
#include <pthread.h>

void *_Exit_1_1_a_thread_func()
{
    printf("entry _Exit_1_1_a_thread_func\n");
    while(1) {
        printf("runing _Exit_1_1_a_thread_func\n");
        sleep(1);
    }
    return NULL;
}

void *_Exit_1_1_b_thread_func()
{
    printf("entry _Exit_1_1_b_thread_func\n");    
    while(1) {
        printf("runing _Exit_1_1_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

int _Exit_1_1()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, _Exit_1_1_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, _Exit_1_1_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    printf("_Exit_1_1 sleep 2s\n");
    sleep(2);
    _Exit(0);
    printf("_Exit_1_1 can't run here\n");
    return 0;
}