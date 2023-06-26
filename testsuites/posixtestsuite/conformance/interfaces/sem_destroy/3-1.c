/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  majid.awad REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content 
 *  of this license, see the COPYING file at the top level of this 
 *  source tree.
 */

/*
 * Test case vrifies sem_destroy shall destroy on intialized semaphore 
 * upon which no threads are currently blocked.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
// #include <fcntl.h>
#include <pthread.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



sem_t sem_destroy_3_1_psem, sem_destroy_3_1_csem; 
int sem_destroy_3_1_n;

int sem_destroy_3_1()
{
    pthread_t prod, cons; 
   void *sem_destroy_3_1_producer(void *);
   void *sem_destroy_3_1_consumer(void *);
   long cnt = 3;

   sem_destroy_3_1_n = 0;
   if (sem_init(&sem_destroy_3_1_csem, 0, 0) < 0) {
        perror("sem_init");
        return PTS_UNRESOLVED;
   }
    if (sem_init(&sem_destroy_3_1_psem, 0, 1) < 0) {
        perror("sem_init");
        return PTS_UNRESOLVED;
   }
   if (pthread_create(&prod, NULL, sem_destroy_3_1_producer, (void *)cnt) != 0) {
        perror("pthread_create");
        return PTS_UNRESOLVED;
   }

     // 增加调度点
	sched_yield();

   if (pthread_create(&cons, NULL, sem_destroy_3_1_consumer, (void *)cnt) != 0) {
        perror("pthread_create");
        return PTS_UNRESOLVED;
   }

     // 增加调度点
     sched_yield();

   if (( pthread_join(prod, NULL) == 0) && ( pthread_join(cons, NULL) == 0)) {
	puts("TEST PASS");
     // TODO: uniproton差异pthread_exit退出后就直接退出了
	// pthread_exit(NULL);
   	if (( sem_destroy(&sem_destroy_3_1_psem) == 0) &&( sem_destroy (&sem_destroy_3_1_csem)) == 0 )
		return PTS_PASS;
   	} else {
		puts("TEST FAILED");
	   	return PTS_FAIL;
   	}
}        


void * sem_destroy_3_1_producer(void *arg)
{
    int i, cnt;
    cnt = (long)arg;
    for (i=0; i<cnt; i++) {
            sem_wait(&sem_destroy_3_1_psem);
            sem_destroy_3_1_n++;  
            sem_post(&sem_destroy_3_1_csem);
    }
    return NULL;
}       

void * sem_destroy_3_1_consumer(void *arg)
{
    int i, cnt;
    cnt = (long)arg;
    for (i=0; i<cnt; i++) {
           sem_wait(&sem_destroy_3_1_csem);
           sem_post(&sem_destroy_3_1_psem);
    }
    return NULL;
}

