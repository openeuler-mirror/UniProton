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
 * The new thread inherits is signal mask from the calling thread, 
 * and has no pending signal at creation time.
 
 * The steps are:
 * 1.  In main(), create a signal mask with a few signals in the set (SIGUSR1 and SIGUSR2).
 * 2.  Raise those signals in main.  These signals now should be pending.
 * 3.  Create a thread using pthread_create().
 * 4.  The thread should have the same signal mask, but no signals should be pending.

 * Parts of this test are copied from 8-1.c (author: rolla.n.selbak REMOVE-THIS AT intel DOT com)
 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <string.h>
 #include <unistd.h>

 #include <sched.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <assert.h>
 #include <sys/wait.h>
 #include <signal.h>
 
 
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
  * void pthread_create_output_init()
  * void pthread_create_output(char * string, ...)
  * 
  * Those may be used to pthread_create_output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * pthread_create_scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of pthread_create_scenarii
 * pthread_create_scenar_init(): function to call before use the pthread_create_scenarii array.
 * pthread_create_scenar_fini(): function to call after end of use of the pthread_create_scenarii array.
 */


/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

typedef struct
{
	sigset_t mask;
	sigset_t pending;
} pthread_create_8_2_testdata_t;


/* Thread function; which will check the signal mask and pending signals */
void * pthread_create_8_2_threaded(void * arg)
{
	int ret;
	pthread_create_8_2_testdata_t * td=(pthread_create_8_2_testdata_t *)arg;
	
	/* Obtain the signal mask of this thread. */
	ret = pthread_sigmask(SIG_SETMASK, NULL, &(td->mask));
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to get the signal mask of the thread");  }
	
	/* Obtain the pending signals of this thread. It should be empty. */
	ret = sigpending(&(td->pending));
	if (ret != 0)  {  UNRESOLVED(errno, "Failed to get pending signals from the thread");  }
	
	/* Signal we're done (especially in case of a detached thread) */
	do { ret = sem_post(&pthread_create_scenarii[pthread_create_sc].sem); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to wait for the semaphore");  }
	
	/* return */
	return arg;
}

int pthread_create_8_2 (int argc, char *argv[])
{
	int ret=0;
	pthread_t child;
	
	pthread_create_8_2_testdata_t td_parent, td_thread;
	
	/* Initialize pthread_create_output routine */
	pthread_create_output_init();
	
	/* Initialize thread attribute objects */
	pthread_create_scenar_init();
	
	/* Initialize the signal state */
	ret = sigemptyset(&(td_parent.mask));
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to initialize a signal set");  }
	
	ret = sigemptyset(&(td_parent.pending));
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to initialize a signal set");  }
	
	/* Add SIGCONT, SIGUSR1 and SIGUSR2 to the set of blocked signals */
	ret = sigaddset(&(td_parent.mask), SIGUSR1);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to add SIGUSR1 to the signal set");  }

	ret = sigaddset(&(td_parent.mask), SIGUSR2);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to add SIGUSR2 to the signal set");  }
	
	/* Block those signals. */
	ret = pthread_sigmask(SIG_SETMASK, &(td_parent.mask), NULL);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to mask the singals in main");  }
	
	/* Raise those signals so they are now pending. */	
	ret = raise(SIGUSR1);
	if (ret != 0)  {  UNRESOLVED(errno, "Failed to raise SIGUSR1");  }
	ret = raise(SIGUSR2);
	if (ret != 0)  {  UNRESOLVED(errno, "Failed to raise SIGUSR2");  }

	/* Do the testing for each thread */
	for (pthread_create_sc=0; pthread_create_sc < NSCENAR; pthread_create_sc++)
	{
		#if VERBOSE > 0
		pthread_create_output("-----\n");
		pthread_create_output("Starting test with scenario (%i): %s\n", pthread_create_sc, pthread_create_scenarii[pthread_create_sc].descr);
		#endif
		
		/* (re)initialize thread signal sets */
		ret = sigemptyset(&(td_thread.mask));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to initialize a signal set");  }
		
		ret = sigemptyset(&(td_thread.pending));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to initialize a signal set");  }
		
		
		ret = pthread_create(&child, &pthread_create_scenarii[pthread_create_sc].ta, pthread_create_8_2_threaded, &td_thread);
		switch (pthread_create_scenarii[pthread_create_sc].result)
		{
			case 0: /* Operation was expected to succeed */
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to create this thread");  }
				break;
			
			case 1: /* Operation was expected to fail */
				if (ret == 0)  {  UNRESOLVED(-1, "An error was expected but the thread creation succeeded");  }
				break;
			
			case 2: /* We did not know the expected result */
			default:
				#if VERBOSE > 0
				if (ret == 0)
					{ pthread_create_output("Thread has been created successfully for this scenario\n"); }
				else
					{ pthread_create_output("Thread creation failed with the error: %s\n", strerror(ret)); }
				#endif
		}
		if (ret == 0) /* The new thread is running */
		{
			if (pthread_create_scenarii[pthread_create_sc].detached == 0)
			{
				ret = pthread_join(child, NULL);
				if (ret != 0)  {  UNRESOLVED(ret, "Unable to join a thread");  }
			}
			else
			{
				/* Just wait for the thread to terminate */
				do { ret = sem_wait(&pthread_create_scenarii[pthread_create_sc].sem); }
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1)  {  UNRESOLVED(errno, "Failed to wait for the semaphore");  }
			}
			
			/* The thread has terminated its work, so we can now control */
			ret = sigismember(&(td_thread.mask), SIGUSR1);
			if (ret != 1)
			{
				if (ret == 0)  {  FAILED("The thread did not inherit the signal mask");  }
				/* else */
				UNRESOLVED(ret, "sigismember() failed"); 
			}
			
			ret = sigismember(&(td_thread.mask), SIGUSR2);
			if (ret != 1)
			{
				if (ret == 0)  {  FAILED("The thread did not inherit the signal mask");  }
				/* else */
				UNRESOLVED(ret, "sigismember() failed"); 
			}

			ret = sigismember(&(td_thread.pending), SIGUSR1);
			if (ret != 0)
			{
				if (ret == 1)  {  FAILED("The thread inherited the pending signal SIGUSR1");  }
				/* else */
				UNRESOLVED(ret, "sigismember() failed"); 
			}
			
			ret = sigismember(&(td_thread.pending), SIGUSR2);
			if (ret != 0)
			{
				if (ret == 1)  {  FAILED("The thread inherited the pending signal SIGUSR2");  }
				/* else */
				UNRESOLVED(ret, "sigismember() failed"); 
			}
			
			
		}
	}
	
	pthread_create_scenar_fini();
	#if VERBOSE > 0
	pthread_create_output("-----\n");
	pthread_create_output("All test data destroyed\n");
	pthread_create_output("Test PASSED\n");
	#endif
	
	PASSED;
}

