/*
 * unp_waitfor.cc
 *
 *  Created on: 20 нояб. 2023 г.
 *      Author: kudr
 */

#ifdef __QNXNTO__

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/netmgr.h>

#include "unp_private.h"

#include "unp_waitfor.h"

#define PREFIX  "WaitFor: "

#define ASSERT(v, prefix) \
	if(!(v)) { \
		l.AddSysError(errno, prefix); \
		throw errno; \
	}

#define PULSE_CODE_PATHSPACE	_PULSE_CODE_MINAVAIL

namespace UNP {

Waitfor::Waitfor() {
	try {
		chId = ChannelCreate(_NTO_CHF_FIXED_PRIORITY);
		ASSERT(chId != -1, PREFIX "ChannelCreate")
		
		coId = ConnectAttach(ND_LOCAL_NODE, 0, chId, _NTO_SIDE_CHANNEL, 0);
		ASSERT(coId != -1, PREFIX "ConnectAttach")
		
		SIGEV_PULSE_INIT(&event, coId, SIGEV_PULSE_PRIO_INHERIT, PULSE_CODE_PATHSPACE, 0);
		notifyId = procmgr_event_notify_add(PROCMGR_EVENT_PATHSPACE, &event);
		ASSERT(notifyId != -1, PREFIX "procmgr_event_notify_add")
	}
	catch(...) {
		Release();
		throw;
	}
}

Waitfor::~Waitfor() {
	Release();
}

void Waitfor::Release() {
	if(notifyId != -1) {
		procmgr_event_notify_delete(notifyId);
		notifyId = -1;
	}
	
	if(coId != -1) {
		if(ConnectDetach(coId) == -1)
			l.AddSysError(errno, PREFIX "ConnectDetach");
		coId = -1;
	}
	
	if(chId != -1) {
		if(ChannelDestroy(chId) == -1)
			l.AddSysError(errno, PREFIX "ChannelDestroy");
		chId = -1;
	}
}

void Waitfor::WaitProcmgr(const char *path, bool appear) {
	if(!path)
		path = "";
	
	for(;; WaitProcmgr()) {
		int r = access(path, F_OK);
		bool ok = r == 0;
		
		if(r < 0 && errno != ENOENT && errno != ENXIO)
			l.AddError("Error access() to '%s': %s", path, strerror(errno));
		
		if(ok == appear)
			break;
	}
}

void Waitfor::WaitProcmgr() {
	struct _pulse pulse;
	int r = MsgReceivePulse(chId, &pulse, sizeof(pulse), NULL);
	ASSERT(r != -1, PREFIX "MsgReceivePulse")
}

}

#endif
