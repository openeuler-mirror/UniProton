/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that the raise(<signal>) function shall send the signal 
 *  to the executing process.
 *  1) Set up a signal handler for the signal that says we have caught the
 *     signal.
 *  2) Raise the signal.
 *  3) If signal handler was called, test passed.
 *  This test is only performed on one signal.  All other signals are
 *  considered to be in the same equivalence class.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

static int handler_flag = 0;

static void handler(int signo)
{
    printf("Caught signal being tested!\n");
    printf("Test PASSED\n");
    handler_flag = 1;
}

int raise_1_1()
{
    struct sigaction act;

    act.sa_handler=handler;
    act.sa_flags=0;
    if (sigemptyset(&act.sa_mask) == -1) {
        perror("Error calling sigemptyset\n");
        return PTS_UNRESOLVED;
    }
    if (sigaction(SIGTOTEST, &act, 0) == -1) {
        perror("Error calling sigaction\n");
        return PTS_UNRESOLVED;
    }
    if (raise(SIGTOTEST) != 0) {
        printf("Could not raise signal being tested\n");
        return PTS_FAIL;
    }

    sleep(1);

    if(handler_flag != 1) {
        printf("Should have exited from signal handler\n");
        printf("Test FAILED\n");
        return PTS_FAIL;
    }

    return PTS_PASS;
}

