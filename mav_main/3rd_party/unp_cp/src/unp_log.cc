/*
 * unp_log.cc
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#include <string.h>

#include "unp_log.h"

namespace UNP {

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::Add(LogMessageType type, const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(type, format, args);
	va_end(args);
}

void Logger::AddError(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_ERROR, format, args);
	va_end(args);
}

void Logger::AddWarning(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_WARNING, format, args);
	va_end(args);
}

void Logger::AddNote(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_NOTE, format, args);
	va_end(args);
}

void Logger::AddHighlight(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_HIGHLIGHT, format, args);
	va_end(args);
}

void Logger::AddSuccess(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_SUCCESS, format, args);
	va_end(args);
}

void Logger::AddDebug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	AddV(LMT_DEBUG, format, args);
	va_end(args);
}

void Logger::AddSysError(int err, const char *prefix) {
	if(prefix)
		AddError("%s: %s (%d)", prefix, strerror(err), err);
	else
		AddError("%s (%d)", strerror(err), err);
}

void Logger::AddSysError(int err, const char *prefix, const char *name) {
	if(!name)
		name = "";
	
	if(prefix)
		AddError("%s '%s': %s (%d)", prefix, name, strerror(err), err);
	else
		AddError("%s '%s' (%d)", name, strerror(err), err);
}

} // namespace
