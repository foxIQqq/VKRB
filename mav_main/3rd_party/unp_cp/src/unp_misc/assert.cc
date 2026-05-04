/*
 * assert.cc
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "unp_misc/assert.h"

void _unp_assert_fail(const char *assertion, const char *file, int line, const char *func) {
	// In function main -- ../../src/main.cc:636 0 == 1 -- assertion failed
	
	int oldCancelState;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
	
	fflush(stderr);
	fprintf(stderr, "In function %s -- %s:%d %s -- assertion failed\n", func, file, line, assertion);
	fflush(stderr);
	
	pthread_setcancelstate(oldCancelState, NULL);
	
	_exit(EXIT_FAILURE);
}
