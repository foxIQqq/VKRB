/*
 * unp.cc
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#include <stdlib.h>

#include "unp_private.h"

#include "unp.h"

namespace UNP {

static Logger *unpLibraryLogger = NULL;
bool unpAssertThrow = true;

bool LibraryInit(Logger &logger, bool assertThrow) {
	unpLibraryLogger = &logger;
	unpAssertThrow = assertThrow; 
	return true;
}

void LibraryFin() {
}

Logger* GetLogger() {
	if(unpLibraryLogger)
		return unpLibraryLogger;
	else {
		static_assert(__cplusplus >= 201103L, ""); // C++11
		
		static class LoggerDefault : public LoggerSys {
		public:
			// Можно попробовать использовать __progname, getprogname()
			LoggerDefault() : LoggerSys("", true, false) {
				unpLibraryLogger = this;
			}
		} loggerDefault;
		
		return unpLibraryLogger;
	}
}

Logger* LibraryInit(const char *appName, bool toStd, bool toJournal, bool assertThrow) {
	static LoggerSys logger(appName, toStd, toJournal);
	return LibraryInit(logger, assertThrow) ? &logger : NULL;
}

}
