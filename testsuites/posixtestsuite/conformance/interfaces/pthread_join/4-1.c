/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
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
* If the thread calling pthread_join is canceled, the joined thread remains joinable.

* The steps are:
* -> create a thread blocked on a mutex.
* -> create another thread which tries and join the first thread.
* -> cancel the 2nd thread.
* -> unblock the semaphore then join the 1st thread

* The test fails if the main thread is unable to join the 1st thread.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
 * void pthread_join_output_init()
 * void pthread_join_output(char * string, ...)
 * 
 * Those may be used to pthread_join_output information.
 */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************     Helper     *****************************************/
/********************************************************************************************/
#include "threads_scenarii.c" 
/* this file defines:
* pthread_join_scenarii: array of struct __pthread_join_scenario type.
* NSCENAR : macro giving the total # of pthread_join_scenarii
* pthread_join_scenar_init(): function to call before use the pthread_join_scenarii array.
* pthread_join_scenar_fini(): function to call after end of use of the pthread_join_scenarii array.
*/

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

pthread_mutex_t pthread_join_4_1_mtx = PTHREAD_MUTEX_INITIALIZER;

/* 1st thread function */
void * pthread_join_4_1_threaded ( void * arg )
{
	int ret = 0;

	/* Try and lock the mutex, then exit */

	ret = pthread_mutex_lock( &pthread_join_4_1_mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to lock mutex" );
	}

	ret = pthread_mutex_unlock( &pthread_join_4_1_mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to unlock mutex" );
	}

	return NULL;
}

/* Canceled thread */
void * pthread_join_4_1_joiner_func( void * arg )
{
	( void ) pthread_join( *( pthread_t * ) arg, NULL );

	FAILED( "The joiner thread was not canceled" );

	/* please the compiler */
	return NULL;
}


/* The main test function. */
int pthread_join_4_1( int argc, char *argv[] )
{
	int ret = 0;
	pthread_t child;
	pthread_t joiner;

	/* Initialize pthread_join_output routine */
	pthread_join_output_init();

	/* Initialize thread attribute objects */
	pthread_join_scenar_init();

	for ( pthread_join_sc = 0; pthread_join_sc < NSCENAR; pthread_join_sc++ )
	{
#if VERBOSE > 0
		pthread_join_output( "-----\n" );
		pthread_join_output( "Starting test with scenario (%i): %s\n", pthread_join_sc, pthread_join_scenarii[ pthread_join_sc ].descr );
#endif

		/* Lock the mutex */
		ret = pthread_mutex_lock( &pthread_join_4_1_mtx );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "failed to lock the mutex" );
		}

		ret = pthread_create( &child, &pthread_join_scenarii[ pthread_join_sc ].ta, pthread_join_4_1_threaded, NULL );

		switch ( pthread_join_scenarii[ pthread_join_sc ].result )
		{
				case 0:                                       /* Operation was expected to succeed */

				if ( ret != 0 )
				{
					UNRESOLVED( ret, "Failed to create this thread" );
				}

				break;

				case 1:                                       /* Operation was expected to fail */

				if ( ret == 0 )
				{
					UNRESOLVED( -1, "An error was expected but the thread creation succeeded" );
				}

				break;

				case 2:                                       /* We did not know the expected result */
				default:
#if VERBOSE > 0

				if ( ret == 0 )
				{
					pthread_join_output( "Thread has been created successfully for this scenario\n" );
				}
				else
				{
					pthread_join_output( "Thread creation failed with the error: %s\n", strerror( ret ) );
				}

#endif

		}

		if ( ret == 0 )                                       /* The new thread is running */
		{

			/* Now create the joiner thread */
			ret = pthread_create( &joiner, NULL, pthread_join_4_1_joiner_func, &child );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to create the joiner thread" );
			}

			/* Let it enter pthread_join */
			sched_yield();

			/* Cancel the joiner thread */
			ret = pthread_cancel( joiner );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to cancel the thread" );
			}

			/* Join the canceled thread */
			ret = pthread_join( joiner, NULL );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to join the canceled thread" );
			}

			/* Unblock the child thread */
			ret = pthread_mutex_unlock( &pthread_join_4_1_mtx );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to unlock the mutex" );
			}

			/* Check the first thread is still joinable */
			ret = pthread_join( child, NULL );

			if ( ret != 0 )
			{
				pthread_join_output( "Error returned: %d\n" );
				FAILED( "The thread is no more joinable" );
			}

		}
		else
		{
			ret = pthread_mutex_unlock( &pthread_join_4_1_mtx );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to unlock the mutex" );
			}
		}
	}

	pthread_join_scenar_fini();
#if VERBOSE > 0
	pthread_join_output( "-----\n" );
	pthread_join_output( "All test data destroyed\n" );
#endif
	pthread_join_output( "Test PASSED\n" );

	PASSED;
}


