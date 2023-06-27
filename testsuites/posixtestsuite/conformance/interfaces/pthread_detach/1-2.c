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
 * pthread_detach() will indicate that the thread resources 
 * can be reclaimed as soon as the thread terminates. 
 * This means that pthread_join() will fail on such a thread.
 
 * The steps are:
 *
 * -> create a thread with a joinable state
 * -> detach the thread, either from inside the thread or from outside
 * -> try and join the thread, and check an error is returned.
 
 * The test fails if pthread_join succeeds.
 
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
  * void pthread_detach_output_init()
  * void pthread_detach_output(char * string, ...)
  * 
  * Those may be used to pthread_detach_output information.
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
 * pthread_detach_scenarii: array of struct __pthread_detach_scenario type.
 * NSCENAR : macro giving the total # of pthread_detach_scenarii
 * pthread_detach_scenar_init(): function to call before use the pthread_detach_scenarii array.
 * pthread_detach_scenar_fini(): function to call after end of use of the pthread_detach_scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

void * pthread_detach_1_2_threaded (void * arg)
{
	int ret=0;
	
	if (arg != NULL)
	{
		ret = pthread_detach(pthread_self());
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to detach the thread");  }
	}
	
	/* Post the semaphore to unlock the main thread in case of a detached thread */
	do { ret = sem_post(&(pthread_detach_scenarii[pthread_detach_sc].sem)); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }
	
	return arg;
}

int pthread_detach_1_2 (int argc, char *argv[])
{
	int ret=0;
	pthread_t child;

	pthread_detach_output_init();
	
	pthread_detach_scenar_init();
	
	for (pthread_detach_sc=0; pthread_detach_sc < NSCENAR; pthread_detach_sc++)
	{
		#if VERBOSE > 0
		pthread_detach_output("-----\n");
		pthread_detach_output("Starting test with scenario (%i): %s\n", pthread_detach_sc, pthread_detach_scenarii[pthread_detach_sc].descr);
		#endif
		
		if (pthread_detach_scenarii[pthread_detach_sc].detached != 0) /* only joinable threads can be detached */
		{
			ret = pthread_attr_setdetachstate(&pthread_detach_scenarii[pthread_detach_sc].ta, PTHREAD_CREATE_JOINABLE);
			if (ret != 0)  {  UNRESOLVED(ret, "Unable to set detachstate back to joinable");  }			
		}
		
		/* for detached pthread_detach_scenarii, we will call pthread_detach from inside the thread.
		   for joinable pthread_detach_scenarii, we'll call pthread_detach from this thread. */
		
		ret = pthread_create(&child, &pthread_detach_scenarii[pthread_detach_sc].ta, pthread_detach_1_2_threaded, (pthread_detach_scenarii[pthread_detach_sc].detached != 0)?&ret:NULL);
		switch (pthread_detach_scenarii[pthread_detach_sc].result)
		{
			case 0: /* Operation was expected to succeed */
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to create this thread");  }
				break;
			
			case 1: /* Operation was expected to fail */
				if (ret == 0)  {  UNRESOLVED(-1, "An error was expected but the thread creation succeeded");  }
			#if VERBOSE > 0
				break;
			
			case 2: /* We did not know the expected result */
			default:
				if (ret == 0)
					{ pthread_detach_output("Thread has been created successfully for this scenario\n"); }
				else
					{ pthread_detach_output("Thread creation failed with the error: %s\n", strerror(ret)); }
			#endif
		}
		if (ret == 0) /* The new thread is running */
		{
			/* Just wait for the thread to terminate */
			do { ret = sem_wait(&(pthread_detach_scenarii[pthread_detach_sc].sem)); }
			while ((ret == -1) && (errno == EINTR));
			if (ret == -1)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }

			/* If we must detach from here, we do it now. */
			if (pthread_detach_scenarii[pthread_detach_sc].detached == 0)
			{
				ret = pthread_detach(child);
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to detach the child thread.");  }
			}
			
			/* now check that the thread resources are freed. */
			ret = pthread_join(child, NULL);
			if (ret == 0)  {  FAILED("We were able to join a detached thread.");  }
			
			/* Let the thread an additionnal row to cleanup */
			sched_yield();
		}
	}
	
	pthread_detach_scenar_fini();
	#if VERBOSE > 0
	pthread_detach_output("-----\n");
	pthread_detach_output("All test data destroyed\n");
	pthread_detach_output("Test PASSED\n");
	#endif
	
	PASSED;
}

