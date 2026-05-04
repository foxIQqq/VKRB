/*
 * unp_irq.cc
 *
 *  Created on: 19.04.2016
 *      Author: kudr
 */

#ifdef __QNXNTO__

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <atomic.h>
#include <sys/neutrino.h>
#include <sys/trace.h>

#include "unp_private.h"
#include "unp_time.h"

#include "unp_irq.h"

#define PREFIX "irq:"

#define ASSERT_RET(v, prefix) \
	if(!(v)) { \
		l.AddSysError(errno, prefix); \
		return false; \
	}

namespace UNP {

Irq::Irq(bool verbose_) : intId(-1), irq(0), verbose(verbose_), chId(-1), coId(-1), intMasked(false), intClearT_us(0), intClearDtMin_us(0) {
	bzero(&isrEvent, sizeof(isrEvent));
}

Irq::~Irq() {
	Release();
}

bool Irq::Init(int irq_, int pulsePrio, int classSizeof, bool flagEnd, bool flagProcess) {
	int r;
	
	r = ThreadCtl(_NTO_TCTL_IO, NULL);
	if(r == -1) {
		l.AddSysError(errno, PREFIX "ThreadCtl _NTO_TCTL_IO");
		return false;
	}
	
	Release();
	
	irq = irq_;
	
	chId = ChannelCreate(0);
	ASSERT_RET(chId != -1, PREFIX "ChannelCreate")
	
	coId = ConnectAttach(0, 0, chId, _NTO_SIDE_CHANNEL, 0);
	ASSERT_RET(coId != -1, PREFIX "ConnectAttach")
	
	SIGEV_PULSE_INIT(&isrEvent, coId, pulsePrio, _PULSE_CODE_MINAVAIL, 0);
	
	unsigned flags = _NTO_INTR_FLAGS_TRK_MSK;
	
	if(flagEnd)
		flags |= _NTO_INTR_FLAGS_END;
	if(flagProcess)
		flags |= _NTO_INTR_FLAGS_PROCESS;
	
	intId = InterruptAttach(irq, Isr_st, this, classSizeof, flags);
	ASSERT_RET(intId != -1, PREFIX "InterruptAttach")
	
	return true;
}

void Irq::Release() {
	if(intId != -1) {
		if(InterruptDetach(intId) == -1)
			l.AddSysError(errno, PREFIX "InterruptDetach");
		intId = -1;
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

const struct sigevent* Irq::Isr() {
	if(verbose)
		trace_logf(1000, "+> %s", __PRETTY_FUNCTION__);
	
	InterruptMask(irq, intId);
	return &isrEvent;
}

const struct sigevent* Irq::Isr_st(void *arg, int /*id*/) {
	Irq *thiz = static_cast<Irq*>(arg);
	return thiz->Isr();
}

void Irq::Unmask() {
	if(intMasked) {
		IntDelay();
		
		InterruptUnmask(irq, intId);
		intMasked = false;
	}
}

bool Irq::Wait() {
	int r;
	_pulse pulse;
	
	Unmask();
	
	r = MsgReceivePulse(chId, &pulse, sizeof(pulse), NULL);
	ASSERT_RET(r == 0, PREFIX "MsgReceivePulse")
	
	pthread_testcancel();
	
	intMasked = MaskedInIsr();
	return true;
}

bool Irq::WaitTimeout(uint64_t timeout_ns, bool *isTimeout) {
	int r;
	_pulse pulse;
	
	if(isTimeout)
		*isTimeout = false;
	
	Unmask();
	
	sigevent timeoutEvent;
	SIGEV_UNBLOCK_INIT(&timeoutEvent);
	
	r = TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_RECEIVE, &timeoutEvent, &timeout_ns, NULL);
	ASSERT_RET(r == 0, PREFIX "TimerTimeout")
	
	r = MsgReceivePulse(chId, &pulse, sizeof(pulse), NULL);
	if(r != 0) {
		if(errno == ETIMEDOUT && isTimeout) {
			*isTimeout = true;
		}
		else {
			l.AddSysError(errno, PREFIX "MsgReceivePulse");
		}
		
		return false;
	}
	
	pthread_testcancel();
	
	intMasked = MaskedInIsr();
	return true;
}

void Irq::SetIntCleared(int64_t dtMinClearUnmask_us) {
	intClearT_us = UNP::GetTime(UNP::TT_MONOTONIC, UNP::TM_US);
	intClearDtMin_us = dtMinClearUnmask_us;
}

void Irq::IntDelay() {
	if(intClearT_us > 0 && intClearDtMin_us > 0) {
		// Должно быть минимальное время между сбросом прерывания и демаскированием
		
		int64_t dt_us = UNP::GetTime(UNP::TT_MONOTONIC, UNP::TM_US) - intClearT_us;
		
		if(dt_us < intClearDtMin_us) {
			usleep(intClearDtMin_us - dt_us);
		}
		
		intClearT_us = 0;
	}
}

}
#endif
