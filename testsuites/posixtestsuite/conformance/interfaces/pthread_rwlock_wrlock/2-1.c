/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * Test pthread_rwlock_wrlock(pthread_rwlock_t * pthread_rwlock_wrlock_2_1_rwlock)
 * 
 * If a signal is delivered to a thread waiting for a read-write lock for writing, upon
 * return from the signal handler the thread resumes waiting for the read-write lock for 
 * writing as if it was not interrupted.
 *
 * Steps:
 * 1. main thread  create read-write lock 'pthread_rwlock_wrlock_2_1_rwlock', and lock it for writing
 * 2. main thread create a thread pthread_rwlock_wrlock_2_1_sig_thread, the thread is set to handle SIGUSR1
 * 3. pthread_rwlock_wrlock_2_1_sig_thread try to lock 'pthread_rwlock_wrlock_2_1_rwlock' for writing but blocked
 * 4. main thread send SIGUSR1 to pthread_rwlock_wrlock_2_1_sig_thread via pthread_kill, while pthread_rwlock_wrlock_2_1_sig_thread is blocking
 * 5. test that thread handler is called 
 * 6. check that when thread handler returns, pthread_rwlock_wrlock_2_1_sig_thread resume block
 * 7. main thread unlock 'pthread_rwlock_wrlock_2_1_rwlock', pthread_rwlock_wrlock_2_1_sig_thread should get the lock
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_t pthread_rwlock_wrlock_2_1_sig_thread;
static pthread_rwlock_t pthread_rwlock_wrlock_2_1_rwlock;

/* pthread_rwlock_wrlock_2_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static int pthread_rwlock_wrlock_2_1_thread_state;
static int pthread_rwlock_wrlock_2_1_handler_called;

static void pthread_rwlock_wrlock_2_1_sig_handler() {
	if(pthread_equal(pthread_self(), pthread_rwlock_wrlock_2_1_sig_thread))
	{
		printf("pthread_rwlock_wrlock_2_1_sig_handler: handled signal SIGUSR1\n");
		pthread_rwlock_wrlock_2_1_handler_called = 1;
	}
	else
	{
		printf("signal was not handled by pthread_rwlock_wrlock_2_1_sig_thread\n");
		return PTS_UNRESOLVED ;
	}
}

static void * pthread_rwlock_wrlock_2_1_th_fn(void *arg)
{
	struct sigaction act;
	int rc = 0;

	/* Set up signal handler for SIGUSR1 */
	act.sa_flags = 0;
	act.sa_handler = pthread_rwlock_wrlock_2_1_sig_handler;
	/* block all the signal while handling SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);
	
	pthread_rwlock_wrlock_2_1_thread_state = ENTERED_THREAD;
	printf("pthread_rwlock_wrlock_2_1_sig_thread: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&pthread_rwlock_wrlock_2_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: pthread_rwlock_wrlock_2_1_sig_thread: Error at pthread_rwlock_wrlock(), error code:%d\n", rc);
		return PTS_FAIL ;
	}

	printf("pthread_rwlock_wrlock_2_1_sig_thread: acquired write lock\n");
	printf("pthread_rwlock_wrlock_2_1_sig_thread: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_wrlock_2_1_rwlock) != 0)
	{
		printf("pthread_rwlock_wrlock_2_1_sig_thread: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;	
	}
 	pthread_rwlock_wrlock_2_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int pthread_rwlock_wrlock_2_1()
{
	int cnt;
	int rc = 0;
	pthread_rwlock_wrlock_2_1_handler_called=0;

	if(pthread_rwlock_init(&pthread_rwlock_wrlock_2_1_rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&pthread_rwlock_wrlock_2_1_rwlock);
	if(rc != 0)
	{
		printf("main: Error at pthread_rwlock_wrlock(), error code:%d\n", rc);
		return PTS_UNRESOLVED;
	}

	pthread_rwlock_wrlock_2_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&pthread_rwlock_wrlock_2_1_sig_thread, NULL, pthread_rwlock_wrlock_2_1_th_fn, NULL) != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* wait at most 3 seconds for pthread_rwlock_wrlock_2_1_sig_thread to block*/
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_wrlock_2_1_thread_state != EXITING_THREAD && cnt++ < 3);
	
	if(pthread_rwlock_wrlock_2_1_thread_state == EXITING_THREAD)
	{
		/* the pthread_rwlock_wrlock_2_1_sig_thread is not blocking*/
		printf("Test FAILED: the thread should block when getting write lock\n");
		return PTS_FAIL ;		
	}
	else if(pthread_rwlock_wrlock_2_1_thread_state != ENTERED_THREAD) 
	{
		printf("pthread_rwlock_wrlock_2_1_sig_thread in unexpected state %d\n", pthread_rwlock_wrlock_2_1_thread_state);
		return PTS_UNRESOLVED ;
	}

	/* pthread_rwlock_wrlock_2_1_sig_thread is blocking */
	printf("main: fire SIGUSR1 to pthread_rwlock_wrlock_2_1_sig_thread\n");
	if(pthread_kill(pthread_rwlock_wrlock_2_1_sig_thread, SIGUSR1) != 0)
	{
		printf("Error at pthread_kill()\n");
		return PTS_UNRESOLVED ;
	}

	/* wait at most 3 seconds for the singal to be handled */		
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_wrlock_2_1_handler_called == 0 && cnt++ < 3);
	
	if(pthread_rwlock_wrlock_2_1_handler_called != 1)
	{
		printf("The signal handler did not get called.\n");
		return PTS_UNRESOLVED ;
	}

	/* pthread_rwlock_wrlock_2_1_sig_thread resume to block? */
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_wrlock_2_1_thread_state != EXITING_THREAD && cnt++ < 3);	

	if(pthread_rwlock_wrlock_2_1_thread_state == 3)
	{
		printf("Test FAILED: upon return from signal handler, pthread_rwlock_wrlock_2_1_sig_thread does not resume to wait\n");
		return PTS_FAIL ;
	}

	printf("pthread_rwlock_wrlock_2_1_sig_thread: correctly still blocking after signal handler returns\n");
	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_wrlock_2_1_rwlock) != 0)
	{
		printf("main: Error releasing write lock\n");
		return PTS_UNRESOLVED ;
	}
	
	/* pthread_rwlock_wrlock_2_1_sig_thread should get write lock */
	cnt = 0;
	do{
		sleep(1);
	}while(pthread_rwlock_wrlock_2_1_thread_state != EXITING_THREAD && cnt++ < 3);
	
	if(pthread_rwlock_wrlock_2_1_thread_state != EXITING_THREAD)
	{
		/* pthread_rwlock_wrlock_2_1_sig_thread does not unblock */
		printf("Test FAILED: pthread_rwlock_wrlock_2_1_sig_thread should get the write lock and exit\n");
		return PTS_FAIL ;	
	}
	
	if(pthread_join(pthread_rwlock_wrlock_2_1_sig_thread, NULL) != 0)
	{
		printf("Error at pthread_join()\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_rwlock_destroy(&pthread_rwlock_wrlock_2_1_rwlock) != 0)
	{
		printf("pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED ;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


