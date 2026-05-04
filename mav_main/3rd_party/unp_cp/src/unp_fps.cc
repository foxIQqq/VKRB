/*
 * fps.cc
 *
 *  Created on: 19.11.2014
 *      Author: kudr
 */

#include <stdio.h>

#include "unp.h"
#include "unp_time.h"
#include "unp_threads.h"

#include "unp_fps.h"

namespace UNP {

static Logger *logStd = NULL;
static pthread_once_t logOnceControl = PTHREAD_ONCE_INIT;

static void LogStdInit() {
	logStd = new UNP::LoggerSys("", true, false);
}

void Fps::CalcPrint(int deltaFramesCount) {
	int64_t tCur = UNP::GetTime(UNP::TT_MONOTONIC, UNP::TM_US), dt;
	
	if(t != 0) {
		dt = tCur - t;
		n += deltaFramesCount;
		if(dt >= 3000000) {
			Logger *l;
			double fps = n * 1000000.0 / dt;
			
			if(printType == PRINT_STD) {
				pthread_once(&logOnceControl, LogStdInit);
				l = logStd;
			}
			else {
				l = UNP::GetLogger();
			}
			
			if(prefix.empty())
				l->AddNote("FPS (%d): %.0f", c, fps);
			else
				l->AddNote("FPS [%s] (%d): %.0f", prefix.c_str(), c, fps);
			
			++c;
			t = tCur;
			n = 0;
		}
	}
	else {
		t = tCur;
		n = 0;
	}
}

}
