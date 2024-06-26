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

 
 * This sample test aims to check the following assertion:
 *
 * If the mutex type is PTHREAD_MUTEX_RECURSIVE, 
 * then the mutex maintains the concept of a lock count.
 * When a thread successfully acquires a mutex for the first time, 
 * the lock count is set to one. Every time a thread relocks this mutex, 
 * the lock count is incremented by one. 
 * Each time the thread unlocks the mutex, 
 * the lock count is decremented by one. 
 * When the lock count reaches zero, 
 * the mutex becomes available and others threads can acquire it. 
 
  
 * The steps are:
 * ->Create a mutex with recursive attribute
 * ->Create a threads
 * ->Parent locks the mutex twice, unlocks once.
 * ->Child attempts to lock the mutex.
 * ->Parent unlocks the mutex.
 * ->Parent unlocks the mutex (shall fail)
 * ->Child unlocks the mutex.
 */ 
 
 /* 
  * - adam.li@intel.com 2004-05-13
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/ 
  *   for general information
  */
 
  /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L

 /* We need the setrlimit() function from X/OPEN standard */ 
 #ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
 
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdarg.h>
 #include <semaphore.h> /* for synchronization */
 #include <errno.h>
 
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
  * void pthread_mutex_lock_output_init()
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
pthread_mutex_t pthread_mutex_lock_4_1_mtx;
sem_t  pthread_mutex_lock_4_1_sem;

/** child thread function **/
void * pthread_mutex_lock_4_1_threaded(void * arg)
{
	int ret;
	/* Try to lock the mutex once. The call must fail here. */
	ret = pthread_mutex_trylock(&pthread_mutex_lock_4_1_mtx);
	if (ret == 0)
	{  FAILED("Child first trylock succeeded");  }
	#if VERBOSE >1
	printf("[thrd] Try to lock the mutex.... failed (normal)\n");
	#endif
	
	/* Free the parent thread and lock the mutex (must success)*/
	if ((ret = sem_post(&pthread_mutex_lock_4_1_sem)))
	{  UNRESOLVED(errno, "1st post pthread_mutex_lock_4_1_sem in child failed");  }
	
	if ((ret = pthread_mutex_lock(&pthread_mutex_lock_4_1_mtx)))
	{  UNRESOLVED(ret, "Child lock failed");  }
	#if VERBOSE >1
	printf("[thrd] Successfully locked the mutex\n");
	#endif
	
	/* Wait for the parent to let us go on */
	if ((ret = sem_post(&pthread_mutex_lock_4_1_sem)))
	{  UNRESOLVED(errno, "2nd post pthread_mutex_lock_4_1_sem in child failed");  }
	
	/* Unlock and exit */
	if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx)))
	{  UNRESOLVED(ret, "Unlock in child failed");  }

	#if VERBOSE >1
	printf("[thrd] Unlocked the mutex, ready to terminate.\n");
	#endif
	
	return NULL;
}

/** parent thread function **/
int pthread_mutex_lock_4_1(int argc, char * argv[])
{
	int ret;
	int i;
	pthread_mutexattr_t ma;
	pthread_t  child;

	pthread_mutex_lock_output_init();

	#if VERBOSE >1
	printf("Initialize the PTHREAD_MUTEX_RECURSIVE mutex\n");
	#endif
	
	/* Initialize the semaphore */
	if ((ret = sem_init(&pthread_mutex_lock_4_1_sem, 0, 0)))
	{  UNRESOLVED(ret, "Sem init failed");  }
	
	/* We initialize the recursive mutex */
	if ((ret = pthread_mutexattr_init(&ma)))
	{  UNRESOLVED(ret, "Mutex attribute init failed");  }
	
	if ((ret = pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE)))
	{  UNRESOLVED(ret, "Set type RECURSIVE failed");  }
	
	if ((ret = pthread_mutex_init(&pthread_mutex_lock_4_1_mtx, &ma)))
	{  UNRESOLVED(ret, "Recursive mutex init failed");  }
	
	if ((ret = pthread_mutexattr_destroy(&ma)))
	{  UNRESOLVED(ret, "Mutex attribute destroy failed");  }
	
	/* -- The mutex is now ready for testing -- */
	
	/* First, we lock it twice and unlock once */
	if ((ret = pthread_mutex_lock(&pthread_mutex_lock_4_1_mtx)))
	{ UNRESOLVED(ret, "First lock failed");  }
	
	if ((ret = pthread_mutex_lock(&pthread_mutex_lock_4_1_mtx)))
	{ FAILED("Second lock failed");  }


	
	if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx)))
	{ FAILED("First unlock failed");  }

	#if VERBOSE >1
	printf("The mutex has been locked twice and unlocked once, start the thread now.\n");
	#endif
	
	/* Here this thread owns the mutex and the internal count is "1" */
	
	/* We create the child thread */
	if ((ret = pthread_create(&child, NULL, pthread_mutex_lock_4_1_threaded, NULL)))
	{  UNRESOLVED(ret, "Unable to create child thread");  }
	
	/* then wait for child to be ready */
	if ((ret = sem_wait(&pthread_mutex_lock_4_1_sem)))
	{  UNRESOLVED(errno, "Wait pthread_mutex_lock_4_1_sem in child failed");  }

	#if VERBOSE >1
	printf("[main] unlock the mutex.\n");
	#endif
	
	/* We can now unlock the mutex */
	if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx)))
	{ FAILED("Second unlock failed");  }

	/* We wait for the child to lock the mutex */
	if ((ret = sem_wait(&pthread_mutex_lock_4_1_sem)))
	{  UNRESOLVED(errno, "Wait pthread_mutex_lock_4_1_sem in child failed");  }

	/* Then, try to unlock the mutex (owned by the child or unlocked) */
	ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx);
	if (ret == 0)
	{ FAILED("Unlock of unowned mutex succeeds");  }
	
	/* Everything seems OK here */
	if ((ret = pthread_join(child, NULL)))
	{  UNRESOLVED(ret, "Child join failed");  }
	
	/* Simple loop to double-check */
	#if VERBOSE >1
	printf("[main] joined the thread.\n");
	printf("Lock & unlock the mutex 50 times.\n");
	#endif

	for (i=0; i<50; i++)
	{
		if ((ret = pthread_mutex_lock(&pthread_mutex_lock_4_1_mtx)))
		{  FAILED("Lock failed in loop");  }
	}
	for (i=0; i<50; i++)
	{
		if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx)))
		{  FAILED("Unlock failed in loop");  }
	}
	
	ret = pthread_mutex_unlock(&pthread_mutex_lock_4_1_mtx);
	if (ret == 0)
	{  FAILED("Unlock succeeds after the loop");  }
	
	#if VERBOSE >1
	printf("Everything went OK; destroy the mutex.\n");
	#endif
	/* The test passed, we destroy the mutex */
	if ((ret = pthread_mutex_destroy(&pthread_mutex_lock_4_1_mtx)))
	{  UNRESOLVED(ret, "Final mutex destroy failed");  }
	
	PASSED;
}
#else /* WITHOUT_XOPEN */
int pthread_mutex_lock_4_1(int argc, char * argv[])
{
	pthread_mutex_lock_output_init();
	UNTESTED("This test requires XSI features");
}
#endif
