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
* pthread_join() returns only when the joined thread has terminated. 

* The steps are:
* -> create a thread
* -> the thread yields then read clock and exit
* -> check the read clock value is before pthread_join return.

* The test fails if the time read is not coherent

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

#include <time.h>
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

/* thread function */
void * pthread_join_1_2_threaded ( void * arg )
{
	int ret = 0;
	int i;
	/* yield the control some times */

	for ( i = 0; i < 10; i++ )
		sched_yield();

	/* Now tell we're done */
	ret = clock_gettime( CLOCK_REALTIME, arg );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to get clock time" );
	}

	return NULL;
}

/* The main test function. */
int pthread_join_1_2( int argc, char *argv[] )
{
	int ret = 0;
	pthread_t child;

	struct timespec ts_pre, ts_th, ts_post;

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
		ret = clock_gettime( CLOCK_REALTIME, &ts_pre );

		if ( ret != 0 )
		{
			UNRESOLVED( errno, "Failed to read clock" );
		}

		ret = pthread_create( &child, &pthread_join_scenarii[ pthread_join_sc ].ta, pthread_join_1_2_threaded, &ts_th );

		switch ( pthread_join_scenarii[ pthread_join_sc ].result )
		{
				case 0:                        /* Operation was expected to succeed */

				if ( ret != 0 )
				{
					UNRESOLVED( ret, "Failed to create this thread" );
				}

				break;

				case 1:                        /* Operation was expected to fail */

				if ( ret == 0 )
				{
					UNRESOLVED( -1, "An error was expected but the thread creation succeeded" );
				}

				break;

				case 2:                        /* We did not know the expected result */
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

		if ( ret == 0 )                        /* The new thread is running */
		{

			ret = pthread_join( child, NULL );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to join a thread" );
			}

			ret = clock_gettime( CLOCK_REALTIME, &ts_post );

			if ( ret != 0 )
			{
				UNRESOLVED( errno, "Failed to read clock" );
			}

			/* Now check that ts_pre <= ts_th <= ts_post */
			if ( ( ts_th.tv_sec < ts_pre.tv_sec )
			        || ( ( ts_th.tv_sec == ts_pre.tv_sec ) && ( ts_th.tv_nsec < ts_pre.tv_nsec ) ) )
			{
				pthread_join_output( "Pre  : %d.%09d\n", ts_pre.tv_sec, ts_pre.tv_nsec );
				pthread_join_output( "child: %d.%09d\n", ts_th.tv_sec, ts_th.tv_nsec );
				pthread_join_output( "Post : %d.%09d\n", ts_post.tv_sec, ts_post.tv_nsec );
				FAILED( "Child returned before its creation ???" );
			}

			if ( ( ts_post.tv_sec < ts_th.tv_sec )
			        || ( ( ts_post.tv_sec == ts_th.tv_sec ) && ( ts_post.tv_nsec < ts_th.tv_nsec ) ) )
			{
				pthread_join_output( "Pre  : %d.%09d\n", ts_pre.tv_sec, ts_pre.tv_nsec );
				pthread_join_output( "child: %d.%09d\n", ts_th.tv_sec, ts_th.tv_nsec );
				pthread_join_output( "Post : %d.%09d\n", ts_post.tv_sec, ts_post.tv_nsec );
				FAILED( "pthread_join returned before child terminated" );
			}

		}
	}

	pthread_join_scenar_fini();
#if VERBOSE > 0
	pthread_join_output( "-----\n" );
	pthread_join_output( "All test data destroyed\n" );
	pthread_join_output( "Test PASSED\n" );
#endif

	PASSED;
}


