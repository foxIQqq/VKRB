/*
 * unp_log_dump.cc
 *
 *  Created on: 29 мар. 2023 г.
 *      Author: kudr
 */

#include <inttypes.h>
#include <string>

#include "unp_private.h"

#include "unp_log.h"

#define LOG_COLOR_PREFIX_MARK  "\033[36;1m"
#define LOG_COLOR_SUFFIX       "\033[0m"

using namespace std;

namespace UNP {

static char HexDigit2Char(uint8_t h) { // h - 4 bit
	if(h <= 9)
		return h + 0x30;
	else {
		switch(h) {
			case 10:
				return 'A';
			case 11:
				return 'B';
			case 12:
				return 'C';
			case 13:
				return 'D';
			case 14:
				return 'E';
			case 15:
				return 'F';
			default:
				return '?';
		}
	}
}

static void LineFlush(const string &lineHex, const string &lineText) {
	l.AddNote("%s| %s", lineHex.c_str(), lineText.c_str());
}

void LogDump(const char *prefix, const char *msg, const void *buf, size_t len) {
	LogDumpDiff(prefix, msg, buf, len, NULL, 0);
}

void LogDumpDiff(const char *prefix, const char *msg, const void *buf_, size_t len, const void *bufPrev_, size_t lenPrev) {
	/* Формат:
	FF FF FF | ...
	*/
	
	const size_t COLUMNS_COUNT = 16;
	
	if(!buf_)
		len = 0;
	
	const uint8_t *buf = static_cast<const uint8_t*>(buf_);
	const uint8_t *bufPrev = static_cast<const uint8_t*>(bufPrev_);
	
	string lineHex, lineText;
	size_t nc, i;
	bool lenChanged = bufPrev && len != lenPrev;
	
	lineHex.reserve(COLUMNS_COUNT * 3);
	lineText.reserve(COLUMNS_COUNT);
	
	l.AddNote(
		"%s%s%s%s(HEX) %s%llu%s bytes:",
		prefix ? prefix : "", prefix && prefix[0] ? ": " : "", msg ? msg : "", msg && msg[0] ? " " : "", lenChanged ? LOG_COLOR_PREFIX_MARK : "", (unsigned long long)len, lenChanged ? LOG_COLOR_SUFFIX : ""
	);
	
	for(i = 0, nc = 0; i < len; ++i) {
		bool changed = bufPrev && i < lenPrev && buf[i] != bufPrev[i];
		
		if(changed)
			lineHex += LOG_COLOR_PREFIX_MARK;
		lineHex += HexDigit2Char((buf[i] >> 4) & 0xf);
		lineHex += HexDigit2Char(buf[i] & 0xf);
		if(changed)
			lineHex += LOG_COLOR_SUFFIX;
		lineHex += ' ';
		
		lineText += isprint(buf[i]) ? buf[i] : '.';
		
		++nc;
		
		if(nc >= COLUMNS_COUNT) {
			LineFlush(lineHex, lineText);
			lineHex.clear();
			lineText.clear();
			nc = 0;
		}
	}
	
	if(nc > 0) {
		for(; nc < COLUMNS_COUNT; ++nc) {
			lineHex += "   ";
			lineText += ' ';
		}
		
		LineFlush(lineHex, lineText);
	}
}

}
