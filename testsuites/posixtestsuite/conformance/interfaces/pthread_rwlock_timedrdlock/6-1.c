/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * Test pthread_rwlock_timedrdlock(pthread_rwlock_t * pthread_rwlock_timedrdlock_6_1_rwlock)
 * 
 * If a signal that causes a signal handler to be executed is delivered to 
 * a thread blocked on a read-write lock via a call to pthread_rwlock_timedrdlock( ),
 * upon return from the signal handler the thread shall resume waiting for the lock 
 * as if it was not interrupted.
 *
 * Test that after returning from a signal handler, the reader will continue
 * to wait with timedrdlock as long as the specified 'timeout' does not expire (the 
 * time spent in signal handler is longer than the specifed 'timeout').
 *
 * Steps:
 * 1. main thread  create and write lock 'pthread_rwlock_timedrdlock_6_1_rwlock'
 * 2. main thread create a thread pthread_rwlock_timedrdlock_6_1_sig_thread, the thread is set to handle SIGUSR1
 * 3. pthread_rwlock_timedrdlock_6_1_sig_thread timed read-lock 'pthread_rwlock_timedrdlock_6_1_rwlock' for reading, it should block
 * 4. While the pthread_rwlock_timedrdlock_6_1_sig_thread is waiting (not expired yet), main thread sends SIGUSR1 
 *    to pthread_rwlock_timedrdlock_6_1_sig_thread via pthread_kill
 * 5. Check that when thread handler returns, pthread_rwlock_timedrdlock_6_1_sig_thread resume block
 * 7. When the wait is terminated, check that the thread wait for a proper period before
 *    expiring. 
 *
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "posixtest.h"

static pthread_t pthread_rwlock_timedrdlock_6_1_sig_thread;
static pthread_rwlock_t pthread_rwlock_timedrdlock_6_1_rwlock;

static int pthread_rwlock_timedrdlock_6_1_thread_state;
static int pthread_rwlock_timedrdlock_6_1_handler_called;
static struct timeval pthread_rwlock_timedrdlock_6_1_before_wait, pthread_rwlock_timedrdlock_6_1_after_wait;


/* pthread_rwlock_timedrdlock_6_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 5

/* Signal handler called by the thread when SIGUSR1 is received */
static void pthread_rwlock_timedrdlock_6_1_sig_handler() {

	if(pthread_equal(pthread_self(), pthread_rwlock_timedrdlock_6_1_sig_thread))
	{
		printf("pthread_rwlock_timedrdlock_6_1_sig_handler: signal is handled by pthread_rwlock_timedrdlock_6_1_sig_thread\n");
		pthread_rwlock_timedrdlock_6_1_handler_called = 1;
		
	}
	else
	{
		printf("pthread_rwlock_timedrdlock_6_1_sig_handler: signal is not handled by pthread_rwlock_timedrdlock_6_1_sig_thread\n");
		return PTS_UNRESOLVED ;
	}
}

static void * pthread_rwlock_timedrdlock_6_1_th_fn(void *arg)
{
	struct sigaction act;
	struct timespec abs_timeout;
	int rc = 0;
	
	pthread_rwlock_timedrdlock_6_1_handler_called = 0;

	/* Set up signal handler for SIGUSR1 */	

	act.sa_flags = 0;
	act.sa_handler = pthread_rwlock_timedrdlock_6_1_sig_handler;
	/* block all the signal when hanlding SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);
	
	gettimeofday(&pthread_rwlock_timedrdlock_6_1_before_wait, NULL);
	abs_timeout.tv_sec = pthread_rwlock_timedrdlock_6_1_before_wait.tv_sec;
	abs_timeout.tv_nsec = pthread_rwlock_timedrdlock_6_1_before_wait.tv_usec * 1000;
	abs_timeout.tv_sec += TIMEOUT;
	
	printf("thread: attempt timed read lock, %d seconds\n", TIMEOUT);
	pthread_rwlock_timedrdlock_6_1_thread_state = ENTERED_THREAD;
	rc = pthread_rwlock_timedrdlock(&pthread_rwlock_timedrdlock_6_1_rwlock, &abs_timeout);
	if(rc != ETIMEDOUT)
	{
		printf("pthread_rwlock_timedrdlock_6_1_sig_thread: pthread_rwlock_timedlock returns %d\n", rc);
		return PTS_FAIL ;
	}
 	printf("thread: timer correctly expired\n");
	gettimeofday(&pthread_rwlock_timedrdlock_6_1_after_wait, NULL);

	pthread_rwlock_timedrdlock_6_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int pthread_rwlock_timedrdlock_6_1()
{
	int cnt;
	struct timeval wait_time;

	if(pthread_rwlock_init(&pthread_rwlock_timedrdlock_6_1_rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_timedrdlock_6_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	pthread_rwlock_timedrdlock_6_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&pthread_rwlock_timedrdlock_6_1_sig_thread, NULL, pthread_rwlock_timedrdlock_6_1_th_fn, NULL) != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* Wait for the thread to get ready for handling signal (the thread should
	 * be block on pthread_rwlock_timedrdlock_6_1_rwlock since main() has the write lock at this point) */	
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_timedrdlock_6_1_thread_state != ENTERED_THREAD && cnt++ < TIMEOUT);
	
	if(pthread_rwlock_timedrdlock_6_1_thread_state != ENTERED_THREAD)
	{
		printf("Unexpected thread state %d\n", pthread_rwlock_timedrdlock_6_1_thread_state);
		return PTS_UNRESOLVED ;
	}

	printf("main: fire SIGUSR1 to thread\n");
	if(pthread_kill(pthread_rwlock_timedrdlock_6_1_sig_thread, SIGUSR1) != 0)
	{
		printf("main: Error at pthread_kill()\n");
		return PTS_UNRESOLVED ;
	}
	
	/* wait at most 2*TIMEOUT seconds */
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_timedrdlock_6_1_thread_state != EXITING_THREAD && cnt++ < 2*TIMEOUT);
	
	if(cnt >= 2*TIMEOUT)
	{
		/* thread blocked*/
		printf("Test FAILED: thread blocked even afer the abs_timeout expired\n");
		return PTS_FAIL ;		
	}
	
	if(pthread_rwlock_timedrdlock_6_1_handler_called != 1)
	{
		printf("The handler for SIGUSR1 did not get called\n");
		return PTS_UNRESOLVED ;
	}	
	
	/* Test that the thread block for the correct TIMOUT time */
	wait_time.tv_sec = pthread_rwlock_timedrdlock_6_1_after_wait.tv_sec - pthread_rwlock_timedrdlock_6_1_before_wait.tv_sec;
	wait_time.tv_usec = pthread_rwlock_timedrdlock_6_1_after_wait.tv_usec - pthread_rwlock_timedrdlock_6_1_before_wait.tv_usec;
	if (wait_time.tv_usec < 0)
	{
		--wait_time.tv_sec;
		wait_time.tv_usec += 1000000;
	}
	if(wait_time.tv_sec < TIMEOUT)
	{
		printf("Test FAILED: Timeout was for %d seconds, but waited for %ld.%06ld seconds instead\n",
			TIMEOUT, (long int)wait_time.tv_sec, (long int)wait_time.tv_usec);
		return PTS_FAIL ;
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_6_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(pthread_rwlock_timedrdlock_6_1_sig_thread, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_destroy(&pthread_rwlock_timedrdlock_6_1_rwlock) != 0)
	{
		printf("Error at pthread_destroy()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


