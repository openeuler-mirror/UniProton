#include <stdlib.h>
#include <pthread.h>

void *quick_exit_1_2_a_thread_func()
{
    printf("entry quick_exit_1_2_a_thread_func\n");
    while(1) {
        printf("runing quick_exit_1_2_a_thread_func\n");
        sleep(1);
        exit(0);
    }
    return NULL;
}

void *quick_exit_1_2_b_thread_func()
{
    printf("entry quick_exit_1_2_b_thread_func\n");    
    while(1) {
        printf("runing quick_exit_1_2_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

int quick_exit_1_2()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, quick_exit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, quick_exit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    printf("quick_exit_1_2 sleep 2s\n");
    sleep(2);
    printf("quick_exit_1_2 can't run here\n");
    return 0;
}