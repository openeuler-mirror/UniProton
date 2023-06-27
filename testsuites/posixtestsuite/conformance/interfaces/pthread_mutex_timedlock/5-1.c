/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_timedlock()
 * 
 * It SHALL fail if:
 * 
 * [EINVAL] - The process or thread would have blocked, and the abs_timeout parameter
 *	     specified in nano-seconds field value is less than 0 or greater than or equal
 * 	     to 1,000 million.
 *
 * Steps: 
 *
 * 1. Create a thread.
 * 2. Call pthread_mutex_timedlock inside of the thread passing to it a negative number in the
 *    nano-seconds field of the 'abs_timeout'.
 * 3. Save the return value of pthread_mutex_timedlock().  It should be EINVAL.
 *
 */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_TIME -1					/* Invalid value of negative value
							   in the nano-seonds field of
							   'abs_timeout. */
#define TIMEOUT 3					/* 3 seconds of timeout time for
							   pthread_mutex_timedlock(). */
void *pthread_mutex_timedlock_5_1_f1(void *parm);

int pthread_mutex_timedlock_5_1_ret;						/* Save return value of 
							   pthread_mutex_timedlock(). */
pthread_mutex_t pthread_mutex_timedlock_5_1_mutex = PTHREAD_MUTEX_INITIALIZER;	/* The pthread_mutex_timedlock_5_1_mutex */
time_t pthread_mutex_timedlock_5_1_currsec1, pthread_mutex_timedlock_5_1_currsec2;				/* Variables for saving time before 
						           and afer locking the pthread_mutex_timedlock_5_1_mutex using
							   pthread_mutex_timedlock(). */	   
/****************************
 *
 * MAIN()
 *
 * *************************/
int pthread_mutex_timedlock_5_1()
{
	pthread_t new_th;

	/* Create a thread that will call pthread_mutex_timedlock */	
	if(pthread_create(&new_th, NULL, pthread_mutex_timedlock_5_1_f1, NULL) != 0)
	{
		perror("Error in pthread_create().\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end. */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join().\n");
		return PTS_UNRESOLVED;
	}

	/* Check the return status of pthread_mutex_timedlock(). */
	if(pthread_mutex_timedlock_5_1_ret != EINVAL)
	{
		printf("Test FAILED: Expected return code EINVAL, got: %d.\n", pthread_mutex_timedlock_5_1_ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

/****************************
 *
 * Thread's start routine.
 * pthread_mutex_timedlock_5_1_f1()
 *
 * *************************/
void *pthread_mutex_timedlock_5_1_f1(void *parm)
{
	struct timespec timeout;
	
	/* Lock the pthread_mutex_timedlock_5_1_mutex */
	if(pthread_mutex_lock(&pthread_mutex_timedlock_5_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_lock()\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

	/* Set nano-seconds to negative value. */
	timeout.tv_sec = time(NULL) + TIMEOUT;
	timeout.tv_nsec = INVALID_TIME;	

	/* This should return EINVAL */
	pthread_mutex_timedlock_5_1_ret = pthread_mutex_timedlock(&pthread_mutex_timedlock_5_1_mutex, &timeout);

	/* Cleaning up the mutexes. */
	if(pthread_mutex_unlock(&pthread_mutex_timedlock_5_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_unlock().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}
	if(pthread_mutex_destroy(&pthread_mutex_timedlock_5_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_destroy().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

  	pthread_exit(0);
  	return (void*)(0);
}
