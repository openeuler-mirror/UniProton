/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_timedwait()
 *   shall return ETIMEDOUT if the time specified by 'abstime' has passed. 
 * 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT   3

static struct testdata
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
} td;

void *pthread_cond_timedwait_4_1_t1_func(void *arg)
{
	int rc;
	struct timespec timeout;
	struct timeval  curtime;
	
	if (pthread_mutex_lock(&td.mutex) != 0) {
		printf("Thread1 failed to acquire the mutex\n");
		pthread_exit((void *)PTS_UNRESOLVED);
	}
	printf("Thread1 started\n");
	
	if (gettimeofday(&curtime, NULL) !=0 ) {
		printf("Fail to get current time\n");
		pthread_exit((void *)PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec + TIMEOUT;
	timeout.tv_nsec = curtime.tv_usec * 1000;

	printf("Thread1 is waiting for the cond for %d seconds\n", TIMEOUT);
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc == ETIMEDOUT) {
		printf("Thread1 stops waiting when time is out\n");
		printf("Test PASSED\n");
		pthread_exit((void *)PTS_PASS);
	}
	else {
		printf("pthread_cond_timedwait return %d instead of ETIMEDOUT\n", rc);
                printf("Test FAILED\n");
		pthread_exit((void *)PTS_FAIL);
        }
}

int pthread_cond_timedwait_4_1()
{
	pthread_t  thread1;
	int th_ret;

	if (pthread_mutex_init(&td.mutex, NULL) != 0) {
		printf("Fail to initialize mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_cond_init(&td.cond, NULL) != 0) {
		printf("Fail to initialize cond\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&thread1, NULL, pthread_cond_timedwait_4_1_t1_func, NULL) != 0) {
		printf("Fail to create thread 1\n");
		return PTS_UNRESOLVED;
	}
	
	printf("Main: no condition is going to be met\n");
	
	pthread_join(thread1, (void*)&th_ret);
    printf("------ th ret = %d --\n", th_ret);
	return th_ret;
}
