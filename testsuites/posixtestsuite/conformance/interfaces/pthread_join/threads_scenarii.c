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


* This file is a helper file for the pthread_join tests
* It defines the following objects:
* pthread_join_scenarii: array of struct __pthread_join_scenario type.
* NSCENAR : macro giving the total # of pthread_join_scenarii
* pthread_join_scenar_init(): function to call before use the pthread_join_scenarii array.
* pthread_join_scenar_fini(): function to call after end of use of the pthread_join_scenarii array.

* It is derived from the file in pthread_create tests, with the detached stuff removed
*
*/


static struct __pthread_join_scenario
{
	/* Object to hold the given configuration, and which will be used to create the threads */
	pthread_attr_t ta;
	/* Scheduling parameters */
	int explicitsched;	/* 0 => sched policy is inherited; 1 => sched policy from the attr param */
	int schedpolicy;	/* 0 => default; 1=> SCHED_FIFO; 2=> SCHED_RR */
	int schedparam;		/* 0 => default sched param; 1 => max value for sched param; -1 => min value for sched param */
	int altscope;		/* 0 => default contension scope; 1 => alternative contension scope */
	/* Stack parameters */
	int altstack;		/* 0 => system manages the stack; 1 => stack is provided */
	int guard;		/* 0 => default guardsize; 1=> guardsize is 0; 2=> guard is 1 page -- this setting only affect system stacks (not user's). */
	int altsize;		/* 0 => default stack size; 1 => stack size specified (min value) -- ignored when stack is provided */
	/* Additionnal information */
	char * descr;		/* object description */
	void * bottom;		/* Stores the stack start when an alternate stack is required */
	int result;		/* This thread creation is expected to: 0 => succeed; 1 => fail; 2 => unknown */
}

pthread_join_scenarii[] =

#define CASE(expl,scp,spa,sco,sta,gua,ssi,desc,res) \
 { \
	 .explicitsched=expl, \
	 .schedpolicy=scp, \
	 .schedparam=spa, \
 	 .altscope=sco, \
 	 .altstack=sta, \
 	 .guard=gua, \
 	 .altsize=ssi, \
 	 .descr=desc, \
 	 .bottom=NULL, \
	 .result=res }

#define CASE_POS(expl,scp,spa,sco,sta,gua,ssi,desc) CASE(expl,scp,spa,sco,sta,gua,ssi,desc,0)
#define CASE_NEG(expl,scp,spa,sco,sta,gua,ssi,desc) CASE(expl,scp,spa,sco,sta,gua,ssi,desc,1)
#define CASE_UNK(expl,scp,spa,sco,sta,gua,ssi,desc) CASE(expl,scp,spa,sco,sta,gua,ssi,desc,2)

        /*
         * This array gives the different combinations of threads attributes for the testcases.
         * 
         * Some combinations must be avoided.
         * -> Do not have a detached thread use an alternative stack; 
         *     as we don't know when the thread terminates to free the stack memory
         * -> ... (to be completed)
         *
         */

        {
                /* Unary tests */
                /* 0*/	 CASE_POS(     0,     0,     0,     0,     0,     0,     0,     "default" )
                /* 1*/	,    CASE_POS(     1,     0,     0,     0,     0,     0,     0,     "Explicit sched" )
                /* 3*/	,    CASE_UNK(     0,     1,     0,     0,     0,     0,     0,     "FIFO Policy" )
                /* 4*/	,    CASE_UNK(     0,     2,     0,     0,     0,     0,     0,     "RR Policy" )
                /* 5*/	,    CASE_UNK(     0,     0,     1,     0,     0,     0,     0,     "Max sched param" )
                /* 6*/	,    CASE_UNK(     0,     0,    -1,     0,     0,     0,     0,     "Min sched param" )
                /* 7*/	,    CASE_POS(     0,     0,     0,     1,     0,     0,     0,     "Alternative contension scope" )
                /* 8*/	,    CASE_POS(     0,     0,     0,     0,     1,     0,     0,     "Alternative stack" )
                /* 9*/	,    CASE_POS(     0,     0,     0,     0,     0,     1,     0,     "No guard size" )
                /*10*/	,    CASE_UNK(     0,     0,     0,     0,     0,     2,     0,     "1p guard size" )
                /*11*/	,    CASE_POS(     0,     0,     0,     0,     0,     0,     1,     "Min stack size" )

                /* Stack play */
                , CASE_POS( 0, 0, 0, 0, 0, 1, 1, "Min stack size, no guard" )
                , CASE_UNK( 0, 0, 0, 0, 0, 2, 1, "Min stack size, 1p guard" )

                /* Scheduling play -- all results are unknown since it might depend on the user priviledges */
                , CASE_UNK( 1, 1, 1, 0, 0, 0, 0, "Explicit FIFO max param" )
                , CASE_UNK( 1, 2, 1, 0, 0, 0, 0, "Explicit RR max param" )
                , CASE_UNK( 1, 1, -1, 0, 0, 0, 0, "Explicit FIFO min param" )
                , CASE_UNK( 1, 2, -1, 0, 0, 0, 0, "Explicit RR min param" )
                , CASE_UNK( 1, 1, 1, 1, 0, 0, 0, "Explicit FIFO max param, alt scope" )
                , CASE_UNK( 1, 2, 1, 1, 0, 0, 0, "Explicit RR max param, alt scope" )
                , CASE_UNK( 1, 1, -1, 1, 0, 0, 0, "Explicit FIFO min param, alt scope" )
                , CASE_UNK( 1, 2, -1, 1, 0, 0, 0, "Explicit RR min param, alt scope" )

        };

#define NSCENAR (sizeof(pthread_join_scenarii) / sizeof(pthread_join_scenarii[0]))

/* This function will initialize every pthread_attr_t object in the pthread_join_scenarii array */
static void pthread_join_scenar_init()
{
	int ret = 0;
	int i;
	int old;
	long pagesize, minstacksize;
	long tsa, tss, tps;

	pagesize	= sysconf( _SC_PAGESIZE );
	minstacksize = sysconf( _SC_THREAD_STACK_MIN );
	tsa	= sysconf( _SC_THREAD_ATTR_STACKADDR );
	tss	= sysconf( _SC_THREAD_ATTR_STACKSIZE );
	tps	= sysconf( _SC_THREAD_PRIORITY_SCHEDULING );

#if VERBOSE > 0
	pthread_join_output( "System abilities:\n" );
	pthread_join_output( " TSA: %li\n", tsa );
	pthread_join_output( " TSS: %li\n", tss );
	pthread_join_output( " TPS: %li\n", tps );
	pthread_join_output( " pagesize: %li\n", pagesize );
	pthread_join_output( " min stack size: %li\n", minstacksize );
#endif

#ifndef __EMSCRIPTEN__
	if ( minstacksize % pagesize )
	{
		UNTESTED( "The min stack size is not a multiple of the page size" );
	}
#endif

	for ( i = 0; i < NSCENAR; i++ )
	{
#if VERBOSE > 2
		pthread_join_output( "Initializing attribute for scenario %i: %s\n", i, pthread_join_scenarii[ i ].descr );
#endif

		ret = pthread_attr_init( &pthread_join_scenarii[ i ].ta );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to initialize a thread attribute object" );
		}

		/* Sched related attributes */
		if ( tps > 0 )     /* This routine is dependent on the Thread Execution Scheduling option */
		{

			if ( pthread_join_scenarii[ i ].explicitsched == 1 )
				ret = pthread_attr_setinheritsched( &pthread_join_scenarii[ i ].ta, PTHREAD_EXPLICIT_SCHED );
			else
				ret = pthread_attr_setinheritsched( &pthread_join_scenarii[ i ].ta, PTHREAD_INHERIT_SCHED );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to set inheritsched attribute" );
			}

#if VERBOSE > 4
			pthread_join_output( "inheritsched state was set sucessfully\n" );

#endif
		}
#if VERBOSE > 4
		else
			pthread_join_output( "TPS unsupported => inheritsched parameter untouched\n" );

#endif

		if ( tps > 0 )     /* This routine is dependent on the Thread Execution Scheduling option */
		{

			if ( pthread_join_scenarii[ i ].schedpolicy == 1 )
			{
				ret = pthread_attr_setschedpolicy( &pthread_join_scenarii[ i ].ta, SCHED_FIFO );
			}

			if ( pthread_join_scenarii[ i ].schedpolicy == 2 )
			{
				ret = pthread_attr_setschedpolicy( &pthread_join_scenarii[ i ].ta, SCHED_RR );
			}

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to set the sched policy" );
			}

#if VERBOSE > 4
			if ( pthread_join_scenarii[ i ].schedpolicy )
				pthread_join_output( "Sched policy was set sucessfully\n" );
			else
				pthread_join_output( "Sched policy untouched\n" );

#endif
		}
#if VERBOSE > 4
		else
			pthread_join_output( "TPS unsupported => sched policy parameter untouched\n" );

#endif

		if ( pthread_join_scenarii[ i ].schedparam != 0 )
		{

			struct sched_param sp;

			ret = pthread_attr_getschedpolicy( &pthread_join_scenarii[ i ].ta, &old );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to get sched policy from attribute" );
			}

			if ( pthread_join_scenarii[ i ].schedparam == 1 )
				sp.sched_priority = sched_get_priority_max( old );

			if ( pthread_join_scenarii[ i ].schedparam == -1 )
				sp.sched_priority = sched_get_priority_min( old );

			ret = pthread_attr_setschedparam( &pthread_join_scenarii[ i ].ta, &sp );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to set the sched param" );
			}

#if VERBOSE > 4
			pthread_join_output( "Sched param was set sucessfully to %i\n", sp.sched_priority );
		}
		else
		{
			pthread_join_output( "Sched param untouched\n" );
#endif

		}

		if ( tps > 0 )     /* This routine is dependent on the Thread Execution Scheduling option */
		{
			ret = pthread_attr_getscope( &pthread_join_scenarii[ i ].ta, &old );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Failed to get contension scope from thread attribute" );
			}

			if ( pthread_join_scenarii[ i ].altscope != 0 )
			{
				if ( old == PTHREAD_SCOPE_PROCESS )
					old = PTHREAD_SCOPE_SYSTEM;
				else
					old = PTHREAD_SCOPE_PROCESS;

				ret = pthread_attr_setscope( &pthread_join_scenarii[ i ].ta, old );

				//if (ret != 0)  {  UNRESOLVED(ret, "Failed to set contension scope");  }
				if ( ret != 0 )
				{
					pthread_join_output( "WARNING: The TPS option is claimed to be supported but setscope fails\n" );
				}

#if VERBOSE > 4
				pthread_join_output( "Contension scope set to %s\n", old == PTHREAD_SCOPE_PROCESS ? "PTHREAD_SCOPE_PROCESS" : "PTHREAD_SCOPE_SYSTEM" );
			}
			else
			{
				pthread_join_output( "Contension scope untouched (%s)\n", old == PTHREAD_SCOPE_PROCESS ? "PTHREAD_SCOPE_PROCESS" : "PTHREAD_SCOPE_SYSTEM" );
#endif

			}

		}
#if VERBOSE > 4
		else
			pthread_join_output( "TPS unsupported => sched contension scope parameter untouched\n" );

#endif

		/* Stack related attributes */
		if ( ( tss > 0 ) && ( tsa > 0 ) )     /* This routine is dependent on the Thread Stack Address Attribute
															                   and Thread Stack Size Attribute options */
		{

			if ( pthread_join_scenarii[ i ].altstack != 0 )
			{
				/* This is slightly more complicated. We need to alloc a new stack
				and free it upon test termination */
				/* We will alloc with a simulated guardsize of 1 pagesize */
				pthread_join_scenarii[ i ].bottom = malloc( minstacksize + pagesize );

				if ( pthread_join_scenarii[ i ].bottom == NULL )
				{
					UNRESOLVED( errno, "Unable to alloc enough memory for alternative stack" );
				}

				ret = pthread_attr_setstack( &pthread_join_scenarii[ i ].ta, pthread_join_scenarii[ i ].bottom, minstacksize );

				if ( ret != 0 )
				{
					UNRESOLVED( ret, "Failed to specify alternate stack" );
				}

#if VERBOSE > 1
				pthread_join_output( "Alternate stack created successfully. Bottom=%p, Size=%i\n", pthread_join_scenarii[ i ].bottom, minstacksize );

#endif

			}

		}
#if VERBOSE > 4
		else
			pthread_join_output( "TSA or TSS unsupported => No alternative stack\n" );

#endif

#ifndef WITHOUT_XOPEN
		if ( pthread_join_scenarii[ i ].guard != 0 )
		{
			if ( pthread_join_scenarii[ i ].guard == 1 )
				ret = pthread_attr_setguardsize( &pthread_join_scenarii[ i ].ta, 0 );

			if ( pthread_join_scenarii[ i ].guard == 2 )
				ret = pthread_attr_setguardsize( &pthread_join_scenarii[ i ].ta, pagesize );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to set guard area size in thread stack" );
			}

#if VERBOSE > 4
			pthread_join_output( "Guard size set to %i\n", ( pthread_join_scenarii[ i ].guard == 1 ) ? 1 : pagesize );

#endif

		}

#endif

		if ( tss > 0 )     /* This routine is dependent on the Thread Stack Size Attribute option */
		{

			if ( pthread_join_scenarii[ i ].altsize != 0 )
			{
				ret = pthread_attr_setstacksize( &pthread_join_scenarii[ i ].ta, minstacksize );

				if ( ret != 0 )
				{
					UNRESOLVED( ret, "Unable to change stack size" );
				}

#if VERBOSE > 4
				pthread_join_output( "Stack size set to %i (this is the min)\n", minstacksize );

#endif

			}

		}
#if VERBOSE > 4
		else
			pthread_join_output( "TSS unsupported => stack size unchanged\n" );

#endif


	}

#if VERBOSE > 0
	pthread_join_output( "All %i thread attribute objects were initialized\n\n", NSCENAR );

#endif
}

/* This function will free all resources consumed in the pthread_join_scenar_init() routine */
static void pthread_join_scenar_fini( void )
{
	int ret = 0, i;

	for ( i = 0; i < NSCENAR; i++ )
	{
		if ( pthread_join_scenarii[ i ].bottom != NULL )
			free( pthread_join_scenarii[ i ].bottom );

		ret = pthread_attr_destroy( &pthread_join_scenarii[ i ].ta );

		if ( ret != 0 )
		{
			UNRESOLVED( ret, "Failed to destroy a thread attribute object" );
		}
	}
}

static int pthread_join_sc = 0; /* This might be very dirty... but is much simpler */

#ifdef STD_MAIN /* We want main to be defined here */

#ifndef THREAD_NAMEED
#define THREAD_NAMEED threaded
#endif

#ifndef RUN_MAIN
#define RUN_MAIN main
#endif

extern void * THREAD_NAMEED(void *arg); /* This is the test function */

int RUN_MAIN (int argc, char *argv[])
{
	int ret = 0;
	pthread_t child;

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

		ret = pthread_create( &child, &pthread_join_scenarii[ pthread_join_sc ].ta, threaded, NULL );

		switch ( pthread_join_scenarii[ pthread_join_sc ].result )
		{
				case 0:     /* Operation was expected to succeed */

				if ( ret != 0 )
				{
					UNRESOLVED( ret, "Failed to create this thread" );
				}

				break;

				case 1:     /* Operation was expected to fail */

				if ( ret == 0 )
				{
					UNRESOLVED( -1, "An error was expected but the thread creation succeeded" );
				}

				break;

				case 2:     /* We did not know the expected result */
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

		if ( ret == 0 )     /* The new thread is running */
		{

			ret = pthread_join( child, NULL );

			if ( ret != 0 )
			{
				UNRESOLVED( ret, "Unable to join a thread" );
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
#endif
