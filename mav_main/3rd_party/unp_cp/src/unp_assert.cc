/*
 * unp_assert.cc
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <stdexcept>

#include "unp.h"
#include "unp_private.h"

#include "unp_assert.h"

namespace UNP {

void AssertFail(const char *format, ...) {
	{
		va_list args;
		
		va_start(args, format);
		l.AddV(LMT_ERROR, format, args);
		va_end(args);
	}
	
	if(unpAssertThrow) {
		va_list args;
		char buf[192];
		
		buf[0] = 0;
		
		va_start(args, format);
		vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		
		throw std::runtime_error(buf);
	}
	else {
		LibraryFin();
		_exit(EXIT_FAILURE);
	}
}

static const char* GetThreadInfo(char *buf, size_t size) {
	pthread_t tid = pthread_self();
#ifdef __QNXNTO__
	char name[32];
	
	name[0] = 0;
	pthread_getname_np(tid, name, sizeof(name));
	
	if(name[0])
		snprintf(buf, size, "id=%d name=%s", int(tid), name);
	else
		snprintf(buf, size, "id=%d", int(tid));
#else
	snprintf(buf, size, "id=%d", int(tid));
#endif
	
	return buf;
}

void AssertSysError(bool cond, int err, const char *prefix) {
	if(!cond) {
		char bufThreadInfo[64];
		bufThreadInfo[0] = 0;
		UNP::AssertFail("Sys call unexpected failed: %s: %s (%d) | thread: %s", prefix ? prefix : "", strerror(err), err, GetThreadInfo(bufThreadInfo, sizeof(bufThreadInfo)));
	}
}

} // namespace

void _UNP_AssertFail(const char *assertion, const char *file, int line, const char *func) {
	// In function main -- ../../src/main.cc:636 0 == 1 -- assertion failed
	
	char bufThreadInfo[64];
	bufThreadInfo[0] = 0;
	UNP::AssertFail("In function %s -- %s:%d %s -- assertion failed | thread: %s", func, file, line, assertion, UNP::GetThreadInfo(bufThreadInfo, sizeof(bufThreadInfo)));
}
