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
*  If the semaphore is locked, the sval value is set to 0 or to a negative 
* value representing the number of waiters for the semaphore. 


* The steps are:
* -> init a semaphore (value = 0)
* -> create a thread which waits for the semaphore
* -> call sem_getvalue and check value of the semaphore
* -> sem_post and destroy everything

* The test fails if value is not as expected.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <errno.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
/******************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c" 
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);  
 *    where descr is a description of the error and ret is an int 
 *   (error code for example)
 * FAILED(descr);
 *    where descr is a short text saying why the test has failed.
 * PASSED();
 *    No parameter.
 * 
 * Both three macros shall terminate the calling process.
 * The testcase shall not terminate in any other maneer.
 * 
 * The other file defines the functions
 * void sem_getvalue_output_init()
 * void sem_getvalue_output(char * string, ...)
 * 
 * Those may be used to sem_getvalue_output information.
 */

/******************************************************************************/
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

void * sem_getvalue_2_2_threaded ( void * arg )
{
	int ret;

	do
	{
		ret = sem_wait( arg );
	}
	while ( ( ret != 0 ) && ( errno == EINTR ) );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to wait for the semaphore" );
	}

	return NULL;
}


/* The main test function. */
int sem_getvalue_2_2( int argc, char * argv[] )
{
	int ret, val;
	sem_t sem;
	pthread_t th;

	/* Initialize sem_getvalue_output */
	sem_getvalue_output_init();

	/* Initialize semaphore */
	ret = sem_init( &sem, 0, 0 );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to init semaphore" );
	}

	/* Create the thread */
	ret = pthread_create( &th, NULL, sem_getvalue_2_2_threaded, &sem );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to create the thread" );
	}

	/* Sleep 1 sec so the thread enters the sem_wait call */
	sleep( 1 );

	/* Check value */
	ret = sem_getvalue( &sem, &val );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to get semaphore value" );
	}

	if ( ( val != 0 ) && ( val != -1 ) )
	{
		printf( "Val: %d\n", val );
		FAILED( "Semaphore count is neither 0 nor # of waiting processes" );
	}

	/* Post the semaphore */
	ret = sem_post( &sem );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to post the semaphore" );
	}

	/* Join the thread */
	ret = pthread_join( th, NULL );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to join the thread" );
	}


	/* Destroy the semaphore */
	ret = sem_destroy( &sem );

	if ( ret != 0 )
	{
		UNRESOLVED( errno, "Failed to sem_destroy" );
	}

	/* Test passed */
#if VERBOSE > 0

	printf( "Test passed\n" );

#endif

	PASSED;
}


