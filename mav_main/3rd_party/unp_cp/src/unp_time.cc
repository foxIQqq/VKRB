/*
 * unp_time.cc
 *
 *  Created on: 21.08.2015
 *      Author: kudr
 */

#include <time.h>
#include <errno.h>
#ifdef __QNX__
#include <sys/neutrino.h>
#include <sys/syspage.h>
#endif

#include "unp_private.h"

#include "unp_time.h"

namespace UNP {

int64_t GetTime(TimeType t, TimeMeasure m) {
	struct timespec tspec;
	
#ifdef __QNX__
	if(t == TT_CLOCK_CYCLES) {
		uint64_t c = ClockCycles(), cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		
		switch(m) {
			case TM_MS:
				return c * 1000 / cps;
			case TM_US:
				return c * 1000000 / cps;
			case TM_NS:
				return c * 1000000000 / cps;
			default: // TM_S
				return c / cps;
		}
	}
	else {
#endif
		if(clock_gettime(t == TT_REAL ? CLOCK_REALTIME : CLOCK_MONOTONIC, &tspec) == 0) {
			int64_t v;
			
			switch(m) {
				case TM_MS:
					v = (int64_t)(tspec.tv_sec) * 1000 + tspec.tv_nsec / 1000000;
					break;
				case TM_US:
					v = (int64_t)(tspec.tv_sec) * 1000000 + tspec.tv_nsec / 1000;
					break;
				case TM_NS:
					v = (int64_t)(tspec.tv_sec) * 1000000000 + tspec.tv_nsec;
					break;
				default: // TM_S
					v = tspec.tv_sec;
					break;
			}
			
			return v;
		}
		else {
			l.AddSysError(errno, "clock_gettime");
			return 0;
		}
#ifdef __QNX__
	}
#endif
}

TimeElapsed::TimeElapsed() : t(UNP::GetTime()) {
}

TimeElapsed::~TimeElapsed() {
	printf("Time elapsed: %lld ms\n", (long long)(UNP::GetTime() - t));
	fflush(stdout);
}

}
