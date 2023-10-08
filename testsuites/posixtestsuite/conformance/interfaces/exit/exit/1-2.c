#include <stdlib.h>
#include <pthread.h>

void *exit_1_2_a_thread_func()
{
    printf("entry exit_1_2_a_thread_func\n");
    while(1) {
        printf("runing exit_1_2_a_thread_func\n");
        sleep(1);
        exit(0);
    }
    return NULL;
}

void *exit_1_2_b_thread_func()
{
    printf("entry exit_1_2_b_thread_func\n");    
    while(1) {
        printf("runing exit_1_2_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

int exit_1_2()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, exit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, exit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    printf("exit_1_2 sleep 2s\n");
    sleep(2);
    return 0;
}