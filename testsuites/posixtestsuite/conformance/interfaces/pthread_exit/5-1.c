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
 * When a thread other than the first thread of a process returns,
 * an implicit call to pthread_exit() is made with the returned value 
 * as a parameter.
 
 * The steps are:
 *
 * Same test as 1-2, 3-2, 4-1, but with return in place of pthread_exit.
 * The results shall be the same.
 
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
  * void pthread_exit_output_init()
  * void pthread_exit_output(char * string, ...)
  * 
  * Those may be used to pthread_exit_output information.
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
 * pthread_exit_scenarii: array of struct __pthread_exit_scenario type.
 * NSCENAR : macro giving the total # of pthread_exit_scenarii
 * pthread_exit_scenar_init(): function to call before use the pthread_exit_scenarii array.
 * pthread_exit_scenar_fini(): function to call after end of use of the pthread_exit_scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

int pthread_exit_5_1_atctl=0;
pthread_key_t pthread_exit_5_1_tld[3];


/* atexit() routines */
void pthread_exit_5_1_at1(void)
{
	pthread_exit_5_1_atctl+=1;
}
void pthread_exit_5_1_at2(void)
{
	pthread_exit_5_1_atctl+=2;
}

/* TLD pthread_exit_5_1_destructor */
void pthread_exit_5_1_destructor(void * arg)
{
	*(int *) arg += 1;
}

/* Thread routine */
void * pthread_exit_5_1_threaded (void * arg)
{
	int ret = 0;
	
	ret = atexit(pthread_exit_5_1_at2);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to register an atexit() routine");  }

	ret = pthread_setspecific(pthread_exit_5_1_tld[0], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	ret = pthread_setspecific(pthread_exit_5_1_tld[1], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	ret = pthread_setspecific(pthread_exit_5_1_tld[2], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	return  NULL + 1;
	
	FAILED("return did not terminate the thread (oO)");
	return NULL;
}

/* Main routine */
int pthread_exit_5_1 (int argc, char *argv[])
{
	int ret=0;
	int ctl=0;
	void * rval;
	pthread_t child;
	int i,j;
	
	pthread_exit_output_init();
	
	pthread_exit_scenar_init();
	
	for (j=0; j<3; j++)
	{
		ret = pthread_key_create(&pthread_exit_5_1_tld[j], pthread_exit_5_1_destructor);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to create a TLD key");  }
	}
	
	for (i=0; i < NSCENAR; i++)
	{
		if (pthread_exit_scenarii[i].detached == 0)
		{
			#if VERBOSE > 0
			pthread_exit_output("-----\n");
			pthread_exit_output("Starting test with scenario (%i): %s\n", i, pthread_exit_scenarii[i].descr);
			#endif
			
			ctl=0;
			
			ret = pthread_create(&child, &pthread_exit_scenarii[i].ta, pthread_exit_5_1_threaded, &ctl);
			switch (pthread_exit_scenarii[i].result)
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
						{ pthread_exit_output("Thread has been created successfully for this scenario\n"); }
					else
						{ pthread_exit_output("Thread creation failed with the error: %s\n", strerror(ret)); }
					#endif
			}
			if (ret == 0) /* The new thread is running */
			{
				ret = pthread_join(child, &rval);
				if (ret != 0)  {  UNRESOLVED(ret, "Unable to join a thread");  }
					
				if (rval != (NULL+1))
				{
					FAILED("pthread_join() did not retrieve the pthread_exit() param");
				}
				
				if (pthread_exit_5_1_atctl != 0)  {  FAILED("The function registered with atexit() executed");  }
				
				if (ctl != 3)  {  FAILED("The TLD destructors were not called");  }
			}
		}
	}
	
	for (j=0; j<3; j++)
	{
		ret = pthread_key_delete(pthread_exit_5_1_tld[j]);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to delete a TLD key");  }
	}
	
	pthread_exit_scenar_fini();
	#if VERBOSE > 0
	pthread_exit_output("-----\n");
	pthread_exit_output("All test data destroyed\n");
	pthread_exit_output("Test PASSED\n");
	#endif
	
	PASSED;
}

