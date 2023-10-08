/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 
 
 * This sample test aims to check the following assertion:
 *
 * The macro PTHREAD_MUTEX_INITIALIZER can be used 
 * to initialize mutexes that are statically allocated. 
 * The effect are equivalent to dynamic initialization by a call to 
 * pthread_mutex_init() with parameter attr specified as NULL, 
 * except that no error checks are performed. 
 * 
 * The steps are:
 *  * create two mutexes. One is initialized with NULL attribute, 
 *       the other is statically initialized with the macro PTHREAD_MUTEX_INITIALIZER.
 *  * Compare the following features between the two mutexes:
 *      -> Can it cause / detect a deadlock? (attempt to lock a mutex the thread already owns). 
 *            If detected, do both mutexes cause the same error code?
 *      -> Is an error pthread_mutex_init_3_2_returned when unlocking the mutex in unlocked state? 
 *            When unlocking the mutex owned by another thread?
 * 
 * 
 * The test will pass if the results of each feature are the same for the two mutexes 
 * (making no assumption on what is the default behavior).
 * The test will be unresolved if any initialization fails.
 * The test will fail if a feature differs between the two mutex objects.
 */

 /* 
  * - adam.li@intel.com 2004-05-09
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/ 
  *   for general information
  */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdarg.h>
 #include <stdlib.h>
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_mutex_init_output_init()
  * void printf(char * string, ...)
  * 
  * Those may be used to printf information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif
 
/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

/**** global variables ****/
pthread_mutex_t * pthread_mutex_init_3_2_p_mtx;
int pthread_mutex_init_3_2_retval = 0;
int pthread_mutex_init_3_2_returned = 0;
int pthread_mutex_init_3_2_canceled = 0;
sem_t pthread_mutex_init_3_2_semA, pthread_mutex_init_3_2_semB;

/***** Cancelation handlers  *****/
void pthread_mutex_init_3_2_cleanup_deadlk(void * arg)
{
	pthread_mutex_init_3_2_canceled = 1;
	pthread_mutex_unlock(pthread_mutex_init_3_2_p_mtx);	
}

/***** Threads functions *****/
void * pthread_mutex_init_3_2_deadlk_issue(void * arg)
{
	int ret, tmp;
	
	if ((ret=pthread_mutex_lock(pthread_mutex_init_3_2_p_mtx)))
	{ UNRESOLVED(ret, "First mutex lock in pthread_mutex_init_3_2_deadlk_issue"); }
	pthread_cleanup_push(pthread_mutex_init_3_2_cleanup_deadlk, NULL);
	if ((ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &tmp)))
	{ UNRESOLVED(ret, "Set cancel type in pthread_mutex_init_3_2_deadlk_issue"); }
	if ((ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &tmp)))
	{ UNRESOLVED(ret, "Set cancel state in pthread_mutex_init_3_2_deadlk_issue"); }
	#if VERBOSE >1
	printf("Thread releases the semaphore...\n");
	#endif
	if ((ret = sem_post(&pthread_mutex_init_3_2_semA)))
	{ UNRESOLVED(errno, "Sem_post in pthread_mutex_init_3_2_deadlk_issue"); }
    
    pthread_mutex_init_3_2_returned = 0;
	pthread_mutex_init_3_2_retval = pthread_mutex_lock(pthread_mutex_init_3_2_p_mtx);
	pthread_mutex_init_3_2_returned = 1;

	if ((ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &tmp)))
	{ UNRESOLVED(ret, "Set cancel state in pthread_mutex_init_3_2_deadlk_issue"); }
	pthread_cleanup_pop(0);
	return NULL;
}

void * pthread_mutex_init_3_2_unlock_issue(void * arg)
{
	int ret;
	
	#if VERBOSE >1
	printf("Locking in child...\n");
	#endif
	if ((ret=pthread_mutex_lock(pthread_mutex_init_3_2_p_mtx)))
	{ UNRESOLVED(ret, "First mutex lock in pthread_mutex_init_3_2_unlock_issue"); }

	if ((ret = sem_post(&pthread_mutex_init_3_2_semA)))
	{ UNRESOLVED(errno, "Sem_post in pthread_mutex_init_3_2_unlock_issue"); }

	if ((ret = sem_wait(&pthread_mutex_init_3_2_semB)))
	{ UNRESOLVED(errno, "Sem_wait in pthread_mutex_init_3_2_unlock_issue"); }

	if (pthread_mutex_init_3_2_retval != 0) /* parent thread failed to unlock the mutex) */
	{
		#if VERBOSE >1
		printf("Unlocking in child...\n");
		#endif
		if ((ret=pthread_mutex_unlock(pthread_mutex_init_3_2_p_mtx)))
		{ FAILED("Mutex unlock pthread_mutex_init_3_2_returned an error but mutex is unlocked."); }
	}

	return NULL;
}


/***** main program *****/
int pthread_mutex_init_3_2(int argc, char *argv[])
{
	pthread_mutex_t mtx_null, 
	                             mtx_macro = PTHREAD_MUTEX_INITIALIZER;
	pthread_t thr;
	
	pthread_mutex_t * tab_mutex[2]={&mtx_null, &mtx_macro};
	int tab_res[2][3]={{0,0,0},{0,0,0}};
	
	int ret;
	void * th_ret;
	
	int i;
	
	pthread_mutex_init_output_init();
	#if VERBOSE >1
	printf("Test starting...\n");
	#endif
	
	/* We first initialize the two mutexes. */
	if ((ret=pthread_mutex_init(&mtx_null, NULL)))
	{ UNRESOLVED(ret, "NULL mutex init"); }
	
	
	if ((ret=sem_init(&pthread_mutex_init_3_2_semA, 0, 0)))
	{ UNRESOLVED(errno, "Sem A init"); }
	if ((ret=sem_init(&pthread_mutex_init_3_2_semB, 0, 0)))
	{ UNRESOLVED(errno, "Sem B init"); }

	#if VERBOSE >1
	printf("Data initialized...\n");
	#endif
	
	
	/* OK let's go for the first part of the test : abnormals unlocking */
	
	/* We first check if unlocking an unlocked mutex returns an error. */
	pthread_mutex_init_3_2_retval = pthread_mutex_unlock(tab_mutex[0]);
	ret = pthread_mutex_unlock(tab_mutex[1]);
	#if VERBOSE >0
	printf("Results for unlock issue #1:\n mutex 1 unlocking pthread_mutex_init_3_2_returned %i\n mutex 2 unlocking pthread_mutex_init_3_2_returned %i\n",
				pthread_mutex_init_3_2_retval, ret);
	#endif
	if (ret != pthread_mutex_init_3_2_retval)
	{
		FAILED("Unlocking an unlocked mutex behaves differently.");
	}
	
    /* Now we focus on unlocking a mutex lock by another thread */
	for (i=0; i<2; i++)
	{
		pthread_mutex_init_3_2_p_mtx = tab_mutex[i];
		tab_res[i][0]=0;
		tab_res[i][1]=0;
		tab_res[i][2]=0;
				
		#if VERBOSE >1
		printf("Creating thread (unlock)...\n");
		#endif

		if ((ret = pthread_create(&thr, NULL, pthread_mutex_init_3_2_unlock_issue, NULL)))
		{ UNRESOLVED(ret, "Unlock issue thread create"); }

		// 增加调度点
		sched_yield();
		
		if ((ret = sem_wait(&pthread_mutex_init_3_2_semA)))
		{ UNRESOLVED(errno, "Sem A wait failed for unlock issue."); }
	
		#if VERBOSE >1
		printf("Unlocking in parent...\n");
		#endif
		pthread_mutex_init_3_2_retval = pthread_mutex_unlock(pthread_mutex_init_3_2_p_mtx);
		
		if ((ret = sem_post(&pthread_mutex_init_3_2_semB)))
		{ UNRESOLVED(errno, "Sem B post failed for unlock issue."); }
		
		if ((ret=pthread_join(thr, &th_ret)))
		{ UNRESOLVED(ret, "Join thread"); }

		#if VERBOSE >1
		printf("Thread joined successfully...\n");
		#endif
		
		tab_res[i][0] = pthread_mutex_init_3_2_retval;
	}
	#if VERBOSE >0
	printf("Results for unlock issue #2:\n mutex 1 pthread_mutex_init_3_2_returned %i\n mutex 2 pthread_mutex_init_3_2_returned %i\n",
				tab_res[0][0],tab_res[1][0]);
	#endif
	
	if (tab_res[0][0] != tab_res[1][0])
	{
		FAILED("Unlocking an unowned mutex behaves differently.");
	}

	
	/* We now are going to test the deadlock issue
	 */
	
	/* We start with testing the NULL mutex features */
	for (i=0; i<2; i++)
	{
		pthread_mutex_init_3_2_p_mtx = tab_mutex[i];
		tab_res[i][0]=0;
		tab_res[i][1]=0;
		tab_res[i][2]=0;
				
		#if VERBOSE >1
		printf("Creating thread (deadlk)...\n");
		#endif

		if ((ret = pthread_create(&thr, NULL, pthread_mutex_init_3_2_deadlk_issue, NULL)))
		{ UNRESOLVED(ret, "Deadlk_issue thread create"); }

		// 增加调度点
		sched_yield();
		
		/* Now we are waiting the thread is ready to relock the mutex. */
		if ((ret=sem_wait(&pthread_mutex_init_3_2_semA)))
		{ UNRESOLVED(errno, "Sem wait"); }
		
		/* To ensure thread runs until second lock, we yield here */
		sched_yield();
		
		/* OK, now we cancel the thread */
		pthread_mutex_init_3_2_canceled=0;
		#if VERBOSE >1
		printf("Cancel thread...\n");
		#endif
		if (pthread_mutex_init_3_2_returned ==0)
			if ((ret=pthread_cancel(thr)))
			{ UNRESOLVED(ret, "Cancel thread (pthread_mutex_init_3_2_deadlk_issue)"); }

		#if VERBOSE >1
		printf("Thread pthread_mutex_init_3_2_canceled...\n");
		#endif
		
		if ((ret=pthread_join(thr, &th_ret)))
		{ UNRESOLVED(ret, "Join thread"); }

		#if VERBOSE >1
		printf("Thread joined successfully...\n");
		#endif
		
		tab_res[i][2] = pthread_mutex_init_3_2_retval;
		tab_res[i][1] = pthread_mutex_init_3_2_returned;
		tab_res[i][0] = pthread_mutex_init_3_2_canceled;
	}
	
	/* Now we parse the results */
	#if VERBOSE >0
	printf("Results for deadlock issue:\n mutex 1 \t%s\t%s%i\n mutex 2 \t%s\t%s%i\n",
				tab_res[0][0]?"deadlock" : "no deadlock",
				tab_res[0][1]?"pthread_mutex_init_3_2_returned " : "did not return ",
				tab_res[0][2],
				tab_res[1][0]?"deadlock" : "no deadlock",
				tab_res[1][1]?"pthread_mutex_init_3_2_returned " : "did not return ",
				tab_res[1][2]);
	#endif
	
	if (tab_res[0][0] != tab_res[1][0])
	{ FAILED("One mutex deadlocks, not the other"); }
	
	if (tab_res[0][1] != tab_res[1][1])
	{ UNRESOLVED(tab_res[0][1], "Abnormal situation!"); }
	
	if ((tab_res[0][1] == 1) && (tab_res[0][2] != tab_res[1][2]))
	{ FAILED("The locks pthread_mutex_init_3_2_returned different error codes."); }

	printf("Test PASS\n");
	PASSED;
} 
