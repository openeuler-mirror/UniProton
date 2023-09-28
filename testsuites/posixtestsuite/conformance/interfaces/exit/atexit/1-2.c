#include <stdlib.h>
#include <pthread.h>

static int count = 0;

void *atexit_1_2_a_thread_func()
{
    printf("entry atexit_1_2_a_thread_func\n");
    while(1) {
        printf("runing atexit_1_2_a_thread_func\n");
        sleep(1);
        exit(0);
    }
    return NULL;
}

void *atexit_1_2_b_thread_func()
{
    printf("entry atexit_1_2_b_thread_func\n");    
    while(1) {
        printf("runing atexit_1_2_b_thread_func\n");
        sleep(1);
    }
    return NULL;
}

void atexit_1_2_run_fun(void)
{
    printf("here is ar exit run fun\n");
    count++;
}

int atexit_1_2()
{
    pthread_t new_th;
    
    if(pthread_create(&new_th, NULL, atexit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating1 thread\n");
        return 1;
    }

    if(pthread_create(&new_th, NULL, atexit_1_2_b_thread_func, NULL) < 0)
    {    
        printf("Error creating2 thread\n");
        return 1;
    }

    atexit(atexit_1_2_run_fun);

    printf("atexit_1_2 sleep 2s\n");
    sleep(2);
    return 0;
}