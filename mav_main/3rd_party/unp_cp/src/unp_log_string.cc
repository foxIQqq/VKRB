/*
 * unp_log_string.cc
 *
 *  Created on: 21 июн. 2022 г.
 *      Author: kudr
 */

#include <pthread.h>
#include <string>

#include "unp_log.h"

using namespace std;

namespace UNP {

class LoggerStringImpl {
	string s;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	char buf[1024];

public:
	LoggerStringImpl() {
		buf[0] = 0;
	}
	
	void AddV(LogMessageType type, const char *format, va_list ap);
	
	string GetClear() {
		string s1;
		pthread_mutex_lock(&mutex);
		swap(s1, s);
		pthread_mutex_unlock(&mutex);
		return s1;
	}
};

void LoggerStringImpl::AddV(LogMessageType type, const char *format, va_list ap) {
	pthread_mutex_lock(&mutex);
	
	const char *tag = "[?]";
	
	switch(type) {
		case LMT_DEBUG:
			tag = "[DEBUG] ";
			break;
		case LMT_NOTE:
			tag = "[NOTE] ";
			break;
		case LMT_HIGHLIGHT:
			tag = "[HIGHLIGHT] ";
			break;
		case LMT_SUCCESS:
			tag = "[SUCCESS] ";
			break;
		case LMT_WARNING:
			tag = "[WARNING] ";
			break;
		case LMT_ERROR:
			tag = "[ERROR] ";
			break;
	}
	
	vsnprintf(buf, sizeof(buf), format, ap);
	s += tag;
	s += buf;
	s += '\n';
	
	pthread_mutex_unlock(&mutex);
}

#define IMPL  static_cast<LoggerStringImpl*>(impl)

LoggerString::LoggerString() : impl(new LoggerStringImpl) {
}

LoggerString::~LoggerString() {
	delete IMPL;
}

void LoggerString::AddV(LogMessageType type, const char *format, va_list ap) {
	IMPL->AddV(type, format, ap);
}

string LoggerString::GetClear() {
	return IMPL->GetClear();
}

} // namespace
