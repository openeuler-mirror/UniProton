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
 * If a signal is delivered to a thread waiting for a mutex, 
 * upon return from the signal handler the thread resumes 
 * waiting for the mutex as if it had not been interrupted. 
 
 
 * The steps are:
 * -> Create a child thread
 * -> Child registers a signal handler
 * -> Child tries to lock a mutex owned by another thread
 * -> A signal is sent to the child
 * -> Check that the signal handler executes and then that the thread still 
          waits for the mutex.
 * -> Release the mutex and check that the child takes it.
 * -> Do all of this several times with different mutex attributes
 * 
 * The test shall be considered to FAIL if it hangs!
 * a call to alarm() might eventually be added but this is a problem under high
 * system stress.
 */
 
 /* 
  * - adam.li@intel.com 2004-05-13
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/ 
  *   for general information
  */
 
  /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We enable the following line to have mutex attributes defined */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
 
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <signal.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdarg.h>
 
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
pthread_mutex_t pthread_mutex_lock_5_1_mtx[5];
sem_t pthread_mutex_lock_5_1_semsig, pthread_mutex_lock_5_1_semstart;
int pthread_mutex_lock_5_1_ctrl=0;

/*********  signal handler  **********/
void pthread_mutex_lock_5_1_sighdl(int sig)
{
	if (sem_post(&pthread_mutex_lock_5_1_semsig))
	{ UNRESOLVED(errno, "Sem_post in signal handler"); }
}

/********** thread *********/
void * pthread_mutex_lock_5_1_threaded(void * arg)
{
	int ret, i; 

	/* We register the signal handler */
	struct sigaction sa;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = pthread_mutex_lock_5_1_sighdl;
	if ((ret = sigaction (SIGUSR1, &sa, NULL)))
	{ UNRESOLVED(ret, "Unable to register signal handler"); }

	/* Start the real work */
	for (i=0; i<5; i++) /* We'll do this with the 5 kinds of mutex */
	{
		if (sem_post(&pthread_mutex_lock_5_1_semstart)) /* Tell the father we are ready */
		{ UNRESOLVED(errno, "Sem post in thread"); }
		
		if ((ret = pthread_mutex_lock(&pthread_mutex_lock_5_1_mtx[i]))) /* Attempt to lock the mutex */
		{ UNRESOLVED(ret, "Mutex lock failed in thread"); }
		
		pthread_mutex_lock_5_1_ctrl++; /* Notify the main we have passed the lock */
		
		if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_5_1_mtx[i]))) /* We don't need the mutex anymore */
		{ UNRESOLVED(ret, "Mutex unlock failed in thread"); }
	}
	return NULL;
}


/********* main ********/
int pthread_mutex_lock_5_1 (int argc, char * argv[])
{
	int ret, i, j;
	pthread_t th;
	pthread_mutexattr_t ma[4], *pma[5];
	pma[4]=NULL;

	pthread_mutex_lock_output_init();

	/* Initialize the mutex attributes */	
	for (i=0; i<4; i++)
	{
		pma[i]=&ma[i];
		if ((ret = pthread_mutexattr_init(pma[i])))
		{ UNRESOLVED(ret, "pthread_mutexattr_init"); }
	}
	#ifndef WITHOUT_XOPEN
		if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_NORMAL)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (normal)"); }
		if ((ret = pthread_mutexattr_settype(pma[1], PTHREAD_MUTEX_ERRORCHECK)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (errorcheck)"); }
		if ((ret = pthread_mutexattr_settype(pma[2], PTHREAD_MUTEX_RECURSIVE)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (recursive)"); }
		if ((ret = pthread_mutexattr_settype(pma[3], PTHREAD_MUTEX_DEFAULT)))
		{ UNRESOLVED(ret, "pthread_mutexattr_settype (default)"); }
		#if VERBOSE >1
		printf("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT initialized\n");
		#endif
	#else
		#if VERBOSE > 0
		printf("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT unavailable\n");
		#endif
	#endif

	/* Initialize the 5 mutex */
	for (i=0; i<5; i++)
	{
		if ((ret = pthread_mutex_init(&pthread_mutex_lock_5_1_mtx[i], pma[i])))
		{ UNRESOLVED(ret, "pthread_mutex_init failed")}
		if ((ret = pthread_mutex_lock(&pthread_mutex_lock_5_1_mtx[i])))
		{ UNRESOLVED(ret, "Initial pthread_mutex_lock failed")}
	}

	#if VERBOSE >1
	printf("Mutex objects are initialized\n");
	#endif

	/* We don't need the mutex attribute objects anymore */
	for (i=0; i<4; i++)
	{
		if ((ret = pthread_mutexattr_destroy(pma[i])))
		{ UNRESOLVED(ret, "pthread_mutexattr_destroy"); }
	}
	
	/* Initialize the semaphores */
	if (sem_init(&pthread_mutex_lock_5_1_semsig, 0, 1))
	{ UNRESOLVED(errno, "Sem init (1) failed"); }
	if (sem_init(&pthread_mutex_lock_5_1_semstart, 0, 0))
	{ UNRESOLVED(errno, "Sem init (0) failed"); }

	
	#if VERBOSE >1
	printf("Going to create the child thread\n");
	#endif
	/* Start the child */
	if ((ret = pthread_create(&th, NULL, pthread_mutex_lock_5_1_threaded, NULL)))
	{ UNRESOLVED(ret, "Unable to create the thread"); }
	#if VERBOSE >1
	printf("Child created\n");
	#endif


	/* Monitor the child */
	for (i=0; i<5; i++) /* We will do this for the 5 kinds of mutex */
	{
		if (sem_wait(&pthread_mutex_lock_5_1_semstart))  /* Wait for the thread to be ready */
		{ UNRESOLVED(errno, "Unable to wait for the child"); }

		#if VERBOSE >1
		printf("Child is ready for iteration %i\n", i+1);
		#endif


		pthread_mutex_lock_5_1_ctrl=0; /* init the pthread_mutex_lock_5_1_ctrl var */
		
		/* Send some signals to the thread */
		for (j=0; j<10; j++)
		{
			if ((ret = sem_wait(&pthread_mutex_lock_5_1_semsig)))
			{ UNRESOLVED(errno, "Sem_wait failed from the signal handler"); }

			sched_yield(); /* Let the child do its stuff - might be a nanosleep here*/

			if ((ret = pthread_kill (th, SIGUSR1)))
			{ UNRESOLVED(ret, "Pthread_kill failed"); }
		}
		#if VERBOSE >1
		printf("Child was killed 10 times\n");
		#endif
		
		/* Now check the thread is still waiting for the mutex */
		if (pthread_mutex_lock_5_1_ctrl != 0)
		{
			FAILED("Killed child passed the pthread_mutex_lock without owning it");
		}
		
		#if VERBOSE >1
		printf("Control was OK\n");
		#endif
		
		/* Unlock the mutex so the thread can proceed to the next one */
		if ((ret = pthread_mutex_unlock(&pthread_mutex_lock_5_1_mtx[i])))
		{  UNRESOLVED(ret, "Mutex unlock in main failed"); }
	}

	#if VERBOSE >1
	printf("The test has passed, we are now going to clean up everything.\n");
	#endif

	/* Clean everything: the test has passed */
	if ((ret = pthread_join(th, NULL)))
	{  UNRESOLVED(ret, "Unable to join the child"); }
	
	for (i=0; i<5; i++)
	{
		if ((ret = pthread_mutex_destroy(&pthread_mutex_lock_5_1_mtx[i])))
		{ UNRESOLVED(ret, "Unable to finally destroy a mutex"); }
	}
	
	if (sem_destroy(&pthread_mutex_lock_5_1_semstart))
	{ UNRESOLVED(errno, "Unable to destroy pthread_mutex_lock_5_1_semstart semaphore"); }
	
	if (sem_destroy(&pthread_mutex_lock_5_1_semsig))
	{ UNRESOLVED(errno, "Unable to destroy pthread_mutex_lock_5_1_semsig semaphore"); }

	PASSED;
}
