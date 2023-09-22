/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_timedwrlock(pthread_rwlock_t *pthread_rwlock_timedwrlock_3_1_rwlock)
 *
 *	pthread_rwlock_timedwrlock( ) function shall apply a write lock to the 
 *	read-write lock referenced by pthread_rwlock_timedwrlock_3_1_rwlock as in the pthread_rwlock_wrlock( ) function. 
 *	However, if the lock cannot be acquired without waiting for other threads to 
 *	unlock the lock, this wait shall be terminate when the specified timeout expires. 
 *
 * Steps:
 * 1.  Initialize pthread_rwlock_timedwrlock_3_1_rwlock
 * 2.  Main creats thread0. 
 * 3.  Thread0 does pthread_rwlock_timedwrlock(), should get the lock successfully then unlock.
 * 4.  Main thread locks 'pthread_rwlock_timedwrlock_3_1_rwlock' for reading with pthread_rwlock_rdlock()
 * 5.  Create a child thread, the thread time-locks 'pthread_rwlock_timedwrlock_3_1_rwlock' for writing, 
 *	using pthread_rwlock_timedwrlock(), should block, but when the timer expires, 
 *	that wait will be terminated.
 * 6.  Main thread unlocks 'pthread_rwlock_timedwrlock_3_1_rwlock'
 * 7.  Main thread locks 'pthread_rwlock_timedwrlock_3_1_rwlock' for writing.
 * 8.  Create child thread to lock 'pthread_rwlock_timedwrlock_3_1_rwlock' for writing, using pthread_rwlock_timedwrlock,
 *	 it should block but when the timer expires, the wait will be terminated
 * 8.  Main thread unlocks 'pthread_rwlock_timedwrlock_3_1_rwlock'
 */

/* Test with CLOCK_REALTIME */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

#define TIMEOUT 3

static pthread_rwlock_t pthread_rwlock_timedwrlock_3_1_rwlock;
static int pthread_rwlock_timedwrlock_3_1_thread_state; 
static struct timeval pthread_rwlock_timedwrlock_3_1_currsec1, pthread_rwlock_timedwrlock_3_1_currsec2;
static int pthread_rwlock_timedwrlock_3_1_expired;

/* pthread_rwlock_timedwrlock_3_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_timedwrlock_3_1_fn_wr(void *arg)
{ 
	struct timespec timeout, ts;
	int rc;
	pthread_rwlock_timedwrlock_3_1_thread_state = ENTERED_THREAD;
#ifdef CLOCK_REALTIME
	rc = clock_gettime(CLOCK_REALTIME, &ts);
	if (rc != 0)
	{
		perror("clock_gettime()");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec = ts.tv_sec;
	pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec = ts.tv_nsec / 1000; 
#else
	gettimeofday(&pthread_rwlock_timedwrlock_3_1_currsec1, NULL);
#endif
	/* Absolute time, not relative. */
	timeout.tv_sec = pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec + TIMEOUT;
	timeout.tv_nsec = pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec * 1000;	
	
	printf("thread: attempt timed write lock, %d secs\n", TIMEOUT);	
	rc = pthread_rwlock_timedwrlock(&pthread_rwlock_timedwrlock_3_1_rwlock, &timeout);
	if(rc  == ETIMEDOUT)
	{
		printf("thread: timer pthread_rwlock_timedwrlock_3_1_expired\n");
		pthread_rwlock_timedwrlock_3_1_expired = 1;
	}
	else if(rc == 0)
	{
		printf("thread: acquired write lock\n");
		pthread_rwlock_timedwrlock_3_1_expired = 0;
		printf("thread: unlock write lock\n");
		if(pthread_rwlock_unlock(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
		{
			printf("thread: error release write lock\n");
			return PTS_UNRESOLVED ;
		}
	}
	else
	{
		printf("thread: Error in pthread_rwlock_timedrdlock().\n");
		return PTS_UNRESOLVED ;
	}
	
	/* Get time after the mutex timed out in locking. */
#ifdef CLOCK_REALTIME
	rc = clock_gettime(CLOCK_REALTIME, &ts);
	if (rc != 0)
	{
		perror("clock_gettime()");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_timedwrlock_3_1_currsec2.tv_sec = ts.tv_sec;
	pthread_rwlock_timedwrlock_3_1_currsec2.tv_usec = ts.tv_nsec / 1000; 
#else
	gettimeofday(&pthread_rwlock_timedwrlock_3_1_currsec2, NULL);
#endif
	pthread_rwlock_timedwrlock_3_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_timedwrlock_3_1()
{
	int cnt = 0;
	pthread_t thread0, thread1, thread2;
	
	if(pthread_rwlock_init(&pthread_rwlock_timedwrlock_3_1_rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: create thread0\n");
	pthread_rwlock_timedwrlock_3_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&thread0, NULL, pthread_rwlock_timedwrlock_3_1_fn_wr, NULL) != 0)
	{
		printf("Error creating thread0\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* thread0 should not block at all since no one else has locked pthread_rwlock_timedwrlock_3_1_rwlock */

	cnt = 0;
	pthread_rwlock_timedwrlock_3_1_expired = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedwrlock_3_1_thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
	
	if(pthread_rwlock_timedwrlock_3_1_thread_state == EXITING_THREAD)
	{
		if(pthread_rwlock_timedwrlock_3_1_expired == 1)
		{
			printf("Test FAILED: the timer pthread_rwlock_timedwrlock_3_1_expired\n");
			return PTS_FAIL ;
		}
		else
			printf("thread0 correctly acquired the write lock.\n");
	}
	else if(pthread_rwlock_timedwrlock_3_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread0 incorrectly blocked on timedwrlock\n");
		return PTS_FAIL ;
	}
	else
	{
		printf("Unexpected state for thread0 %d\n", pthread_rwlock_timedwrlock_3_1_thread_state);
		return PTS_UNRESOLVED ;
	}	
	
	if(pthread_join(thread0, NULL) != 0)
	{
		printf("Error when joining thread0\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired read lock\n");
	
	pthread_rwlock_timedwrlock_3_1_thread_state = NOT_CREATED_THREAD;
	
	printf("main: create thread1\n");
	if(pthread_create(&thread1, NULL, pthread_rwlock_timedwrlock_3_1_fn_wr, NULL) != 0)
	{
		printf("Error when creating thread1\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after TIMEOUT*2 seconds,
	   we regard it as blocked */

	/* we expect the thread to expire blocking after timeout*/
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedwrlock_3_1_thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
		
	if(pthread_rwlock_timedwrlock_3_1_thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = pthread_rwlock_timedwrlock_3_1_currsec2.tv_sec - pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec;
		time_diff.tv_usec = pthread_rwlock_timedwrlock_3_1_currsec2.tv_usec - pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec;
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
				printf("Test PASS: the timer expired and thread terminated, "
					"start time %ld.%06ld, end time %ld.%06ld\n", 
					(long) pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec, (long) pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec, 
					(long) pthread_rwlock_timedwrlock_3_1_currsec2.tv_sec, (long) pthread_rwlock_timedwrlock_3_1_currsec2.tv_usec);
			} else {
#endif
			printf("Test FAILED: the timer pthread_rwlock_timedwrlock_3_1_expired and blocking "
				"was terminated, but the timeout is not correct: "
				"expected to wait for %d, but waited for %ld.%06ld\n", 
				TIMEOUT, (long)time_diff.tv_sec, 
				(long)time_diff.tv_usec);
			return PTS_FAIL ;
#if defined(_SIM_)
			}
#endif
		}
		else
			printf("thread1 correctly pthread_rwlock_timedwrlock_3_1_expired at timeout.\n");
	}
	else if(pthread_rwlock_timedwrlock_3_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: wait is not terminated even "
			"when the timer pthread_rwlock_timedwrlock_3_1_expired\n");
		return PTS_FAIL ;
	}
	else
	{
		printf("Unexpected state for thread1 %d\n", pthread_rwlock_timedwrlock_3_1_thread_state);
		return PTS_UNRESOLVED ;
	}
	
	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
	{
		printf("Error when release read lock\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("Error when joining thread1\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;	
	}
	printf("main: acquired write lock\n");

	pthread_rwlock_timedwrlock_3_1_thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create thread2\n");
	if(pthread_create(&thread2, NULL, pthread_rwlock_timedwrlock_3_1_fn_wr, NULL) != 0)
	{
		printf("Error when creating thread2\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* we expect thread2 to expire blocking after timeout */
	do{
		sleep(1);
	}while (pthread_rwlock_timedwrlock_3_1_thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
	
	if(pthread_rwlock_timedwrlock_3_1_thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = pthread_rwlock_timedwrlock_3_1_currsec2.tv_sec - pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec;
		time_diff.tv_usec = pthread_rwlock_timedwrlock_3_1_currsec2.tv_usec - pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec;
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
				printf("Test PASS: the timer expired and thread terminated, "
					"start time %ld.%06ld, end time %ld.%06ld\n", 
					(long) pthread_rwlock_timedwrlock_3_1_currsec1.tv_sec, (long) pthread_rwlock_timedwrlock_3_1_currsec1.tv_usec, 
					(long) pthread_rwlock_timedwrlock_3_1_currsec2.tv_sec, (long) pthread_rwlock_timedwrlock_3_1_currsec2.tv_usec);
			} else {
#endif
			printf("Test FAILED: for thread 2, the timer pthread_rwlock_timedwrlock_3_1_expired "
			"and waiter terminated, but the timeout is not correct\n");
			return PTS_FAIL ;
#if defined(_SIM_)
			}
#endif
		}
		else
			printf("thread2 correctly pthread_rwlock_timedwrlock_3_1_expired at timeout.\n");
		
	}
	else if(pthread_rwlock_timedwrlock_3_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: for thread2, wait is not terminated "
			"even when the timer pthread_rwlock_timedwrlock_3_1_expired\n");
		return PTS_FAIL ;
	}
	else
	{
		printf("Unexpected state for thread2 %d\n", pthread_rwlock_timedwrlock_3_1_thread_state);
		return PTS_UNRESOLVED ;
	}

	printf("main: unlock write lock\n");
	pthread_rwlock_timedwrlock_3_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_rwlock_unlock(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
	{
		printf("Error releasing write lock\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_timedwrlock_3_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
