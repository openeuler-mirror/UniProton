/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  majid.awad REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content 
 *  of this license, see the COPYING file at the top level of this 
 *  source tree.
 */

/*
 *  This test case illustrate the semaphore is shared between processes when
 *  pshared value is non-zero.
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
#define FUNCTION "sem_init"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



sem_t sem_init_3_1_psem, sem_init_3_1_csem; 
int sem_init_3_1_n;

int sem_init_3_1()
{
    pthread_t prod, cons; 
   void *sem_init_3_1_producer(void *);
   void *sem_init_3_1_consumer(void *);
   long cnt = 3;

   sem_init_3_1_n = 0;
   if (sem_init(&sem_init_3_1_csem, 0, 0) < 0) {
        perror("sem_init");
        return PTS_UNRESOLVED;
   }
    if (sem_init(&sem_init_3_1_psem, 0, 1) < 0) {
        perror("sem_init");
        return PTS_UNRESOLVED;
   }
   if (pthread_create(&prod, NULL, sem_init_3_1_producer, (void *)cnt) != 0) {
        perror("pthread_create");
        return PTS_UNRESOLVED;
   }
   if (pthread_create(&cons, NULL, sem_init_3_1_consumer, (void *)cnt) != 0) {
        perror("pthread_create");
        return PTS_UNRESOLVED;
   }

   if (( pthread_join(prod, NULL) == 0) && ( pthread_join(cons, NULL) == 0)) {
	   puts("TEST PASS");
   		sem_destroy(&sem_init_3_1_psem);
		sem_destroy(&sem_init_3_1_csem);
	   return PTS_PASS;
   } else {
	   puts("TEST FAILED");
	   return PTS_FAIL;
   }
}        


void * sem_init_3_1_producer(void *arg)
{
    int i, cnt;
    cnt = (long)arg;
    for (i=0; i<cnt; i++) {
            sem_wait(&sem_init_3_1_psem);
            sem_init_3_1_n++;  
            sem_post(&sem_init_3_1_csem);
    }
    return NULL;
}       

void * sem_init_3_1_consumer(void *arg)
{
    int i, cnt;
    cnt = (long)arg;
    for (i=0; i<cnt; i++) {
           sem_wait(&sem_init_3_1_csem);
           sem_post(&sem_init_3_1_psem);
    }
    return NULL;
}

