/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_timedlock()
 * locks the pthread_mutex_timedlock_2_1_mutex object referenced by 'pthread_mutex_timedlock_2_1_mutex'.  If the pthread_mutex_timedlock_2_1_mutex is
 * already locked, the calling thread shall block until the pthread_mutex_timedlock_2_1_mutex becomes
 * available.  The wait will end when the specified timeout time has expired.

 * The timeout expires when the absolute time 'abs_timeout' passes, or if 'abs_timeout'
 * has already been passed the time of the call.

 * Steps: 
 *
 * 1. Create a pthread_mutex_timedlock_2_1_mutex in the main() thread and lock it.
 * 2. Create a thread, and call pthread_mutex_timedlock inside of it.  It should block for
 *    the set time of (3 secs.).
 * 3. Save the time before and after the thread tried to lock the pthread_mutex_timedlock_2_1_mutex.
 * 4. After the thread has ended, main() will compare the times before and after the pthread_mutex_timedlock_2_1_mutex
 *    tried to lock in the thread.
 */

/* Test for CLOCK_REALTIME */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

#define TIMEOUT 3					/* 3 seconds of timeout time for
							   pthread_mutex_timedlock(). */
void *pthread_mutex_timedlock_2_1_f1(void *parm);

pthread_mutex_t pthread_mutex_timedlock_2_1_mutex = PTHREAD_MUTEX_INITIALIZER;	/* The pthread_mutex_timedlock_2_1_mutex */
struct timeval pthread_mutex_timedlock_2_1_currsec1, pthread_mutex_timedlock_2_1_currsec2;			/* Variables for saving time before 
						           and after locking the pthread_mutex_timedlock_2_1_mutex using
							   pthread_mutex_timedlock(). */	   
/****************************
 *
 * MAIN()
 *
 * *************************/
int pthread_mutex_timedlock_2_1()
{
	pthread_t new_th;
	struct timeval time_diff;

	/* Lock the pthread_mutex_timedlock_2_1_mutex. */
	if(pthread_mutex_lock(&pthread_mutex_timedlock_2_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_lock().\n");
		return PTS_UNRESOLVED;
	}

	/* Create a thread that will call pthread_mutex_timedlock */	
	if(pthread_create(&new_th, NULL, pthread_mutex_timedlock_2_1_f1, NULL) != 0)
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

	/* Cleaning up the mutexes. */
	if(pthread_mutex_unlock(&pthread_mutex_timedlock_2_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_unlock().\n");
		return PTS_UNRESOLVED;
	}
	if(pthread_mutex_destroy(&pthread_mutex_timedlock_2_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_destroy().\n");
		return PTS_UNRESOLVED;
	}

	/* Compare time before the pthread_mutex_timedlock_2_1_mutex locked and after the pthread_mutex_timedlock_2_1_mutex lock timed out. */
	time_diff.tv_sec = pthread_mutex_timedlock_2_1_currsec2.tv_sec - pthread_mutex_timedlock_2_1_currsec1.tv_sec;
	time_diff.tv_usec = pthread_mutex_timedlock_2_1_currsec2.tv_usec - pthread_mutex_timedlock_2_1_currsec1.tv_usec;
	if (time_diff.tv_usec < 0)
	{
		--time_diff.tv_sec;
		time_diff.tv_usec += 1000000;
	}
	if(time_diff.tv_sec < TIMEOUT)
	{
#if defined(_SIM_)
        /* qemu时钟差异较大，gap在10ms内测试通过 */
		if ((1000000 - time_diff.tv_usec)/1000 < 10) {
			printf("time before pthread_mutex_timedlock_2_1_mutex locked: %ld.%06ld, time after pthread_mutex_timedlock_2_1_mutex timed out: %ld.%06ld.\n", (long)pthread_mutex_timedlock_2_1_currsec1.tv_sec, (long)pthread_mutex_timedlock_2_1_currsec1.tv_usec, (long)pthread_mutex_timedlock_2_1_currsec2.tv_sec, (long)pthread_mutex_timedlock_2_1_currsec2.tv_usec);
			printf("Test PASSED\n");
			return PTS_PASS;
		}
#endif
		printf("Test FAILED: Timed lock did not wait long enough. (%d secs.)\n", TIMEOUT);
		printf("time before pthread_mutex_timedlock_2_1_mutex locked: %ld.%06ld, time after pthread_mutex_timedlock_2_1_mutex timed out: %ld.%06ld.\n", (long)pthread_mutex_timedlock_2_1_currsec1.tv_sec, (long)pthread_mutex_timedlock_2_1_currsec1.tv_usec, (long)pthread_mutex_timedlock_2_1_currsec2.tv_sec, (long)pthread_mutex_timedlock_2_1_currsec2.tv_usec);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

/****************************
 *
 * Thread's start routine.
 * pthread_mutex_timedlock_2_1_f1()
 *
 * *************************/
void *pthread_mutex_timedlock_2_1_f1(void *parm)
{
	struct timespec timeout, ts;
	int rc;
	/* Get the current time before the pthread_mutex_timedlock_2_1_mutex locked. */
#ifdef CLOCK_REALTIME
	printf("Test CLOCK_REALTIME\n");
	rc = clock_gettime(CLOCK_REALTIME, &ts);
	if (rc != 0)
	{
		perror("clock_gettime()");
		return PTS_UNRESOLVED ;
	}
	pthread_mutex_timedlock_2_1_currsec1.tv_sec = ts.tv_sec;
	pthread_mutex_timedlock_2_1_currsec1.tv_usec = ts.tv_nsec / 1000; 
#else
	gettimeofday(&pthread_mutex_timedlock_2_1_currsec1, NULL);
#endif
	/* Absolute time, not relative. */
	timeout.tv_sec = pthread_mutex_timedlock_2_1_currsec1.tv_sec + TIMEOUT;
	timeout.tv_nsec = pthread_mutex_timedlock_2_1_currsec1.tv_usec * 1000;	

	printf("Timed pthread_mutex_timedlock_2_1_mutex lock will block for %d seconds starting from: %ld.%06ld\n", TIMEOUT, (long)pthread_mutex_timedlock_2_1_currsec1.tv_sec, (long)pthread_mutex_timedlock_2_1_currsec1.tv_usec);
	if(pthread_mutex_timedlock(&pthread_mutex_timedlock_2_1_mutex, &timeout) != ETIMEDOUT)
	{
		perror("Error in pthread_mutex_timedlock().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

	/* Get time after the pthread_mutex_timedlock_2_1_mutex timed out in locking. */
#ifdef CLOCK_REALTIME
	rc = clock_gettime(CLOCK_REALTIME, &ts);
	if (rc != 0)
	{
		perror("clock_gettime()");
		return PTS_UNRESOLVED ;
	}
	pthread_mutex_timedlock_2_1_currsec2.tv_sec = ts.tv_sec;
	pthread_mutex_timedlock_2_1_currsec2.tv_usec = ts.tv_nsec / 1000; 
#else
	gettimeofday(&pthread_mutex_timedlock_2_1_currsec2, NULL);
#endif
  	pthread_exit(0);
  	return (void*)(0);
}
