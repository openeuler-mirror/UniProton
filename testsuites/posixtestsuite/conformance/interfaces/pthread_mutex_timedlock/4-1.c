/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_timedlock()
 * 
 * Upon success, it returns 0.
 *
 * Steps: 
 *
 * 1. Create a thread, and call pthread_mutex_timedlock inside of it.  It should not block
 *    and should return 0 since it will be the only one owning the pthread_mutex_timedlock_4_1_mutex.
 * 2. Save the return value of pthread_mutex_timedlock().  It should be 0.
 *
 */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT 3					/* 3 seconds of timeout time for
							   pthread_mutex_timedlock(). */
void *pthread_mutex_timedlock_4_1_f1(void *parm);

int pthread_mutex_timedlock_4_1_ret;						/* Save return value of 
							   pthread_mutex_timedlock(). */
pthread_mutex_t pthread_mutex_timedlock_4_1_mutex = PTHREAD_MUTEX_INITIALIZER;	/* The pthread_mutex_timedlock_4_1_mutex */
time_t pthread_mutex_timedlock_4_1_currsec1, pthread_mutex_timedlock_4_1_currsec2;				/* Variables for saving time before 
						           and afer locking the pthread_mutex_timedlock_4_1_mutex using
							   pthread_mutex_timedlock(). */	   
/****************************
 *
 * MAIN()
 *
 * *************************/
int pthread_mutex_timedlock_4_1()
{
	pthread_t new_th;

	/* Create a thread that will call pthread_mutex_timedlock */	
	if(pthread_create(&new_th, NULL, pthread_mutex_timedlock_4_1_f1, NULL) != 0)
	{
		perror("Error in pthread_create().\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();

	/* Wait for thread to end. */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join().\n");
		return PTS_UNRESOLVED;
	}

	/* Check the return status of pthread_mutex_timedlock(). */
	if(pthread_mutex_timedlock_4_1_ret != 0)
	{
		printf("Test FAILED: Expected return code 0, got: %d.\n", pthread_mutex_timedlock_4_1_ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

/****************************
 *
 * Thread's start routine.
 * pthread_mutex_timedlock_4_1_f1()
 *
 * *************************/
void *pthread_mutex_timedlock_4_1_f1(void *parm)
{
	struct timespec timeout;

	// TODO: 支持timeout后不需要
	// timeout.tv_sec = time(NULL) + TIMEOUT;
	if (clock_gettime(CLOCK_REALTIME, &timeout) != 0) {
		perror("clock_gettime()");
		return PTS_UNRESOLVED;
	}

	timeout.tv_sec += TIMEOUT;
	timeout.tv_nsec = 0;	

	/* This should not block since the pthread_mutex_timedlock_4_1_mutex is not owned by anyone right now. 
	 * Save the return value. */
	pthread_mutex_timedlock_4_1_ret = pthread_mutex_timedlock(&pthread_mutex_timedlock_4_1_mutex, &timeout);

	/* Cleaning up the mutexes. */
	if(pthread_mutex_unlock(&pthread_mutex_timedlock_4_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_unlock().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}
	if(pthread_mutex_destroy(&pthread_mutex_timedlock_4_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_destroy().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

  	pthread_exit(0);
  	return (void*)(0);
}
