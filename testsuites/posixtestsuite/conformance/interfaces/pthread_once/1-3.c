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
* Subsequent calls with the same once_control do not call init routine. 

* The steps are:
* -> Create several threads
* -> each call pthread_once
* -> check the init_routine executed once

* The test fails if the init_routine has not been called or has
* been called several times.

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
 * void output_init()
 * void output(char * string, ...)
 * 
 * Those may be used to output information.
 */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NTHREADS 3

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

static int control;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void pthread_once_1_3_my_init( void )
{
	int ret = 0;

	ret = pthread_mutex_lock( &mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to lock mutex in initializer" );
	}

	control++;

	ret = pthread_mutex_unlock( &mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to unlock mutex in initializer" );
	}

	return ;
}

/* Thread function */
void * pthread_once_1_3_threaded ( void * arg )
{
	int ret;

	ret = pthread_once( arg, pthread_once_1_3_my_init );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "pthread_once failed" );
	}

	return NULL;
}

/* The main test function. */
int pthread_once_1_3( int argc, char * argv[] )
{
	int ret, i;

	pthread_once_t myctl = PTHREAD_ONCE_INIT;

	pthread_t th[ NTHREADS ];

	/* Initialize output */
	output_init();

	control = 0;

	/* Create the children */

	for ( i = 0; i < NTHREADS; i++ )
	{
		ret = pthread_create( &th[ i ], NULL, pthread_once_1_3_threaded, &myctl );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to create a thread" );
		}
	}

	/* Then join */
	for ( i = 0; i < NTHREADS; i++ )
	{
		ret = pthread_join( th[ i ], NULL );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to join a thread" );
		}
	}

	/* Fetch the memory */
	ret = pthread_mutex_lock( &mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to lock mutex in initializer" );
	}

	if ( control != 1 )
	{
		printf( "Control: %d\n", control );
		FAILED( "The initializer function did not execute once" );
	}

	ret = pthread_mutex_unlock( &mtx );

	if ( ret != 0 )
	{
		UNRESOLVED( ret, "Failed to unlock mutex in initializer" );
	}

	printf("Test PASS\n");
	PASSED;
}


