#include <stdlib.h>
#include <pthread.h>

static int count = 0;

void *at_quick_exit_1_3_a_thread_func()
{
    printf("entry at_quick_exit_1_3_a_thread_func\n");
    while(1) {
        printf("runing at_quick_exit_1_3_a_thread_func\n");
        sleep(1);
    }
    return NULL;
}

void *at_quick_exit_1_3_b_thread_func()
{
    printf("entry at_quick_exit_1_3_b_thread_func\n");    
    while(1) {
        printf("runing at_quick_exit_1_3_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

void at_quick_exit_1_3_run_fun(void)
{
    printf("here is quick run fun\n");
    count++;
}

int at_quick_exit_1_3()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, at_quick_exit_1_3_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, at_quick_exit_1_3_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    at_quick_exit(at_quick_exit_1_3_run_fun);

    printf("at_quick_exit_1_3 sleep 2s\n");
    sleep(2);
    exit(0);
    printf("at_quick_exit_1_3 can't run here\n");
    return 0;
}