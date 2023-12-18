/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that the sigpending() function stores the set of signals that 
 *  are blocked from delivery to a signal handler function.  Steps are:
 *  1)  Block three signals from delivery to a signal handler.
 *  2)  Raise the signal to get into that signal handler.
 *  From the signal handler:
 *    3)  Raise two of the blocked signals.
 *    4)  Verify that the two signals raised are shown via sigpending.
 *    5)  Verify the one signal not raised is not shown.
 */

/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  The handler raises the two blocked signals and verifies they are
 *  shown via sigpending().
 *  It uses exit() to leave so that the signals are not executed.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

static int handler_flag = 0;

static void handler(int signo)
{
    sigset_t pendingset;

    if (sigemptyset(&pendingset) == -1) {
        printf("Could not call sigemptyset()\n");
    }
    if (raise(SIGCONT) != 0) {
        printf("Could not raise SIGCONT\n");
    }
    if (raise(SIGUSR1) != 0) {
        printf("Could not raise SIGUSR1\n");
    }
    if (sigpending(&pendingset) == -1) {
        printf("Error calling sigpending()\n");
    }
    if (sigismember(&pendingset, SIGCONT) == 1) {
        if (sigismember(&pendingset, SIGUSR1) == 1) {
            printf("All pending signals found\n");
            if (sigismember(&pendingset, SIGHUP) == 0) {
                printf("Unsent signals not found.\n");
                printf("Test PASSED\n");
                handler_flag = 1;
            } else {
                printf("Error with unsent signals\n");
                printf("Test FAILED\n");
            }
        } else {
            printf("Not all pending signals found\n");
        }
    } else {
        printf("Not all pending signals found\n");
    }
}

int sigpending_1_2()
{
        struct sigaction act;

        act.sa_handler = handler;
        act.sa_flags = 0;

    if (sigemptyset(&act.sa_mask) == -1) {
        printf("Could not call sigemptyset()\n");
        return PTS_UNRESOLVED;
    }

    if ( (sigaddset(&act.sa_mask, SIGCONT) == -1) ||
        (sigaddset(&act.sa_mask, SIGHUP) == -1) ||
        (sigaddset(&act.sa_mask, SIGUSR1) == -1) ) {
        perror("Error calling sigaddset()\n");
        return PTS_UNRESOLVED;
    }

    if (sigaction(SIGTTOU, &act, 0) == -1) {
        perror("Could not call sigaction()");
        return PTS_UNRESOLVED;
    }

    if (raise(SIGTTOU) == -1) {
        perror("Could not raise SIGTTOU");
        return PTS_UNRESOLVED;
    }

    sleep(1);

    if (handler_flag != 1) {
        perror("check handler_flag fail");
        return PTS_FAIL;
    }

    sigset_t blockset;
    sigemptyset(&blockset);
    sigprocmask(SIG_SETMASK, &blockset, NULL);

    return PTS_PASS;
}
