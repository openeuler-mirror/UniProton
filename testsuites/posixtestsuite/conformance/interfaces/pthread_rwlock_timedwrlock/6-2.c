/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * Test pthread_rwlock_timedwrlock(pthread_rwlock_t * pthread_rwlock_timedwrlock_6_2_rwlock)
 * 
 * If a signal that causes a signal handler to be executed is delivered to 
 * a thread blocked on a read-write lock via a call to pthread_rwlock_timedwrlock( ),
 * upon return from the signal handler the thread shall resume waiting for the lock 
 * as if it was not interrupted.
 *
 * Steps:
 * 1. main thread  create read-write lock 'pthread_rwlock_timedwrlock_6_2_rwlock', and lock it for writing
 * 2. main thread create a thread pthread_rwlock_timedwrlock_6_2_sig_thread, the thread is set to handle SIGUSR1
 * 3. pthread_rwlock_timedwrlock_6_2_sig_thread timed lock 'pthread_rwlock_timedwrlock_6_2_rwlock' for writing, but blocked
 * 4. While the pthread_rwlock_timedwrlock_6_2_sig_thread is waiting(not expried yet), main thread send SIGUSR1 
 *    to pthread_rwlock_timedwrlock_6_2_sig_thread via pthread_kill
 * 5. test that thread handler is called, inside the handler, make the thread sleep
 *    for a period that the specified 'timeout' for pthread_rwlock_timedwrlock() 
 *    should have pthread_rwlock_timedwrlock_6_2_expired (timeout * 2) 
 * 6. While pthread_rwlock_timedwrlock_6_2_sig_thread sleeping in signal handler, main thread unlock 'pthread_rwlock_timedwrlock_6_2_rwlock' 
 * 7. check that when thread handler returns, pthread_rwlock_timedwrlock_6_2_sig_thread get the read lock without 
 *    getting ETIMEDOUT.
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

/* pthread_rwlock_timedwrlock_6_2_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 2

static pthread_t pthread_rwlock_timedwrlock_6_2_sig_thread;
static pthread_rwlock_t pthread_rwlock_timedwrlock_6_2_rwlock;

static int pthread_rwlock_timedwrlock_6_2_thread_state;
static int pthread_rwlock_timedwrlock_6_2_handler_state;
static int pthread_rwlock_timedwrlock_6_2_expired;
static struct timeval pthread_rwlock_timedwrlock_6_2_before_wait, pthread_rwlock_timedwrlock_6_2_after_wait;

static void pthread_rwlock_timedwrlock_6_2_sig_handler() {

	struct timespec sleep_time_req;
	
	sleep_time_req.tv_sec = TIMEOUT*2;
	sleep_time_req.tv_nsec = 0;
	
	if(pthread_equal(pthread_self(), pthread_rwlock_timedwrlock_6_2_sig_thread))
	{
		printf("pthread_rwlock_timedwrlock_6_2_sig_handler: signal is handled by thread\n");
		/* pthread_rwlock_timedwrlock_6_2_sig_handler will not sleep 2 times more than the timeout for the
		 * pthread_rwlock_timerdlock is waiting for */
		printf("pthread_rwlock_timedwrlock_6_2_sig_handler: sleeping for %d seconds\n", (int)sleep_time_req.tv_sec);	
		pthread_rwlock_timedwrlock_6_2_handler_state = 2;
		sleep((int)sleep_time_req.tv_sec);
	}
	else
	{
		printf("pthread_rwlock_timedwrlock_6_2_sig_handler: signal is not handled by thread\n");
		return PTS_UNRESOLVED ;
	}

	pthread_rwlock_timedwrlock_6_2_handler_state = 3;
}

static void * pthread_rwlock_timedwrlock_6_2_th_fn(void *arg)
{
	struct sigaction act;
	struct timespec abs_timeout;
	int rc;
	pthread_rwlock_timedwrlock_6_2_handler_state = 2;
	pthread_rwlock_timedwrlock_6_2_expired = 0;

	/* Set up handler for SIGUSR1 */
	
	act.sa_flags = 0;
	act.sa_handler = pthread_rwlock_timedwrlock_6_2_sig_handler;
	/* block all the signal when hanlding SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);
	
	gettimeofday(&pthread_rwlock_timedwrlock_6_2_before_wait, NULL);
	abs_timeout.tv_sec = pthread_rwlock_timedwrlock_6_2_before_wait.tv_sec + TIMEOUT;
	abs_timeout.tv_nsec = pthread_rwlock_timedwrlock_6_2_before_wait.tv_usec * 1000;
	
	pthread_rwlock_timedwrlock_6_2_thread_state = ENTERED_THREAD;
	
	printf("thread: attempt timed write lock, %d seconds\n", TIMEOUT);
	rc = pthread_rwlock_timedwrlock(&pthread_rwlock_timedwrlock_6_2_rwlock, &abs_timeout);
	if(rc == 0)
	{
		printf("thread: correctly acquired write lock\n");
		pthread_rwlock_timedwrlock_6_2_expired = 0;	
	}	 
	else if(rc == ETIMEDOUT)
	{
		printf("thread: timer pthread_rwlock_timedwrlock_6_2_expired, did not acquire write lock");
		pthread_rwlock_timedwrlock_6_2_expired = 1;
	}
	else
	{
		printf("Error %d at pthread_rwlock_timedwrlock()\n", rc);
		return PTS_UNRESOLVED ;
	}

	gettimeofday(&pthread_rwlock_timedwrlock_6_2_after_wait, NULL);

	pthread_rwlock_timedwrlock_6_2_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int pthread_rwlock_timedwrlock_6_2()
{
	int cnt;

	if(pthread_rwlock_init(&pthread_rwlock_timedwrlock_6_2_rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_timedwrlock_6_2_rwlock) != 0)
	{
		printf("pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	pthread_rwlock_timedwrlock_6_2_thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&pthread_rwlock_timedwrlock_6_2_sig_thread, NULL, pthread_rwlock_timedwrlock_6_2_th_fn, NULL) != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();

	/* wait for the thread to get ready for handling signal */	
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_timedwrlock_6_2_thread_state != ENTERED_THREAD && cnt++ < TIMEOUT);
	
	if(pthread_rwlock_timedwrlock_6_2_thread_state != ENTERED_THREAD)
	{
		printf("Error: thread did not block when getting write lock\n");
		return PTS_UNRESOLVED ;
	}

	printf("main: fire SIGUSR1 to thread\n");
	if(pthread_kill(pthread_rwlock_timedwrlock_6_2_sig_thread, SIGUSR1) != 0)
	{
		printf("Error in pthread_kill()");
		return PTS_UNRESOLVED ;
	}
	
	/* Wait for signal handler to sleep so that main can unlock the pthread_rwlock_timedwrlock_6_2_rwlock while
	 * it is sleeping. (this way, the pthread_rwlock_timedwrlock_6_2_rwlock will be unlocked when the signal handler
	 * returns, and control is given back to the thread) */

	cnt = 0;
	do{
		sleep(TIMEOUT);
	}while(pthread_rwlock_timedwrlock_6_2_handler_state !=2 && cnt++ < 2);

	if(pthread_rwlock_timedwrlock_6_2_handler_state == 1)
	{
		printf("Error: signal handler did not get called\n");
		return PTS_UNRESOLVED ;
	}
	else if(pthread_rwlock_timedwrlock_6_2_handler_state == 3)
	{
		printf("Error: signal handler incorrectly exited\n");
		return PTS_UNRESOLVED ;	
	}

	if(pthread_rwlock_timedwrlock_6_2_expired == 1)
	{
		printf("Error: thread timeout in pthread_rwlock_timedwrlock_6_2_sig_handler\n");
		return PTS_UNRESOLVED ;
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedwrlock_6_2_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}
	
	/* wait at most 4*TIMEOUT seconds for thread to exit */
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_timedwrlock_6_2_thread_state != EXITING_THREAD && cnt++ < 4*TIMEOUT);
	
	
	if(cnt >= 4*TIMEOUT)
	{
		/* thread blocked*/
		printf("Test FAILED: thread blocked even afer the abs_timeout expires\n");
		return PTS_FAIL ;		
	}
	
	if(pthread_rwlock_timedwrlock_6_2_expired == 1)
	{
		printf("Test FAILED: thread should get the write lock\n");
		return PTS_FAIL ;
	}
	
	if(pthread_join(pthread_rwlock_timedwrlock_6_2_sig_thread, NULL) != 0)
	{
		printf("Error at pthread_join()");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_destroy(&pthread_rwlock_timedwrlock_6_2_rwlock) != 0)
	{
		printf("Error at pthread_destroy()");
		return PTS_UNRESOLVED ;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}
