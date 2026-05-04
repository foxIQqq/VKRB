/*
 * unp_threads.cc
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <time.h>
#include <inttypes.h>

#include "unp_assert.h"
#include "unp_private.h"
#include "unp_misc/assert.h"

#include "unp_threads.h"

#ifdef __QNXNTO__
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#define NS  1000000000LL
#endif

namespace UNP {

bool ThreadCreate(pthread_t *tid, void *(*rtn)(void *), void *rtnArg, size_t stackSize) {
	pthread_attr_t attr;
	int err;
	
	err = pthread_attr_init(&attr);
	if(err != 0) {
		l.AddSysError(err, "pthread_attr_init");
		return false;
	}
	
	if(stackSize <= 0)
		stackSize = UNP_THREAD_STACK_SIZE_DEFAULT;
	
	bool ok = true;
	
	try {
		if(stackSize > 0) { //-V547
			err = pthread_attr_setstacksize(&attr, stackSize);
			if(err != 0) {
				l.AddSysError(err, "pthread_attr_setstacksize");
				throw 1;
			}
		}
		
		if(tid == NULL) {
			err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			if(err != 0) {
				l.AddSysError(err, "pthread_attr_setdetachstate");
				throw 1;
			}
		}
		
		{
			pthread_t tid1;
			err = pthread_create(tid != NULL ? tid : &tid1, &attr, rtn, rtnArg);
			if(err != 0) {
				l.AddSysError(err, "pthread_create");
				throw 1;
			}
		}
	}
	catch(...) {
		ok = false;
	}
	
	pthread_attr_destroy(&attr);
	return ok;
}

Mutex::Mutex() {
	bzero(&mutex, sizeof(mutex));
	
	/*int r = pthread_mutex_init(&mutex, NULL);
	AssertSysError(r == 0, r, "pthread_mutex_init");*/
	
	unp_assert( pthread_mutex_init(&mutex, NULL) == 0 );
}

Mutex::~Mutex() {
	/*int r = pthread_mutex_destroy(&mutex);
	AssertSysError(r == 0, r, "pthread_mutex_destroy");*/
	
	unp_assert( pthread_mutex_destroy(&mutex) == 0 );
}

void Mutex::Lock() {
	int r = pthread_mutex_lock(&mutex);
	AssertSysError(r == 0, r, "pthread_mutex_lock");
}

bool Mutex::Trylock() {
	int r = pthread_mutex_trylock(&mutex);
	if(r == EBUSY)
		return false;
	else {
		AssertSysError(r == 0, r, "pthread_mutex_trylock");
		return true;
	}
}

void Mutex::Unlock() {
	int r = pthread_mutex_unlock(&mutex);
	AssertSysError(r == 0, r, "pthread_mutex_unlock");
}

void Mutex::UnlockSt(void *thiz) {
	static_cast<UNP::Mutex*>(thiz)->Unlock();
}

CondMutex::CondMutex() {
	bzero(&cond, sizeof(cond));
	
//	int r;
	pthread_condattr_t attr;
	
	pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	
	/*r = pthread_cond_init(&cond, &attr);
	AssertSysError(r == 0, r, "pthread_cond_init");*/
    
    unp_assert( pthread_cond_init(&cond, &attr) == 0 );
}

CondMutex::~CondMutex() {
	/*int r = pthread_cond_destroy(&cond);
	AssertSysError(r == 0, r, "pthread_cond_destroy");*/
	
	unp_assert( pthread_cond_destroy(&cond) == 0 );
}

void CondMutex::Wait() {
	int r = pthread_cond_wait(&cond, &mutex);
	AssertSysError(r == 0, r, "pthread_cond_wait");
}

void CondMutex::WaitTimed(const timespec &absTime, bool *isTimeout) {
	int r;
	
	r = pthread_cond_timedwait(&cond, &mutex, &absTime);
	if(r == ETIMEDOUT) {
		*isTimeout = true;
		return;
	}
	else {
		AssertSysError(r == 0, r, "pthread_cond_timedwait");
		*isTimeout = false;
	}
}

void CondMutex::Trigger() {
	int r = pthread_cond_broadcast(&cond);
	AssertSysError(r == 0, r, "pthread_cond_broadcast");
}

void CondMutex::WaitCycle(void *userData, Condition_f conditionFun, bool needUnlock) {
	Lock();
	pthread_cleanup_push(Mutex::UnlockSt, static_cast<Mutex*>(this));
	
	while(conditionFun(userData))
		Wait();
	
	pthread_testcancel(); // т.к. м. не пройти через Wait();
	pthread_cleanup_pop(needUnlock);
}

void CondMutex::WaitCycleTimed(uint64_t timeout_ns, bool *isTimeout, void *userData, Condition_f conditionFun, bool needUnlock) {
	timespec t;
	bool isTo = false;
	
	GetTimeSpec(&t, timeout_ns);
	
	Lock();
	pthread_cleanup_push(Mutex::UnlockSt, static_cast<Mutex*>(this));
	
	while(!isTo && conditionFun(userData))
		WaitTimed(t, &isTo);
	
	pthread_testcancel();
	pthread_cleanup_pop(needUnlock);
	
//	if(isTo)
//		l.AddError("Timeout");
	
	*isTimeout = isTo;
}

void CondMutex::WaitCycleTimed_nc(uint64_t timeout_ns, bool *isTimeout, void *userData, Condition_f conditionFun, bool needUnlock) {
	UNP::ThreadCancelDisabler d;
	timespec t;
	bool isTo = false;
	
	GetTimeSpec(&t, timeout_ns);
	
	Lock();
	
	while(!isTo && conditionFun(userData))
		WaitTimed(t, &isTo);
	
	if(needUnlock)
		Unlock();
	
//	if(isTo)
//		l.AddError("Timeout");
	
	*isTimeout = isTo;
}

Rwlock::Rwlock() {
	unp_assert( pthread_rwlock_init(&rwlock, NULL) == 0 );
}

Rwlock::~Rwlock() {
	unp_assert( pthread_rwlock_destroy(&rwlock) == 0 );
}

void Rwlock::Rdlock() {
	int r = pthread_rwlock_rdlock(&rwlock);
	AssertSysError(r == 0, r, "pthread_rwlock_rdlock");
}

void Rwlock::Wrlock() {
	int r = pthread_rwlock_wrlock(&rwlock);
	AssertSysError(r == 0, r, "pthread_rwlock_wrlock");
}

void Rwlock::Unlock() {
	int r = pthread_rwlock_unlock(&rwlock);
	AssertSysError(r == 0, r, "pthread_rwlock_unlock");
}

void SetCancelState(int state, int *oldstate) {
	int r = pthread_setcancelstate(state, oldstate);
	AssertSysError(r == 0, r, "pthread_setcancelstate");
}

Sem::Sem(unsigned value) {
	bzero(&sem, sizeof(sem));
	
	/*int r = sem_init(&sem, 0, value);
	AssertSysError(r == 0, r, "sem_init");*/
	
	unp_assert( sem_init(&sem, 0, value) == 0 );
}

Sem::~Sem() {
	/*int r = sem_destroy(&sem);
	AssertSysError(r == 0, r, "sem_destroy");*/
	
	unp_assert( sem_destroy(&sem) == 0 );
}

void Sem::Wait() {
	int r = sem_wait(&sem);
	AssertSysError(r == 0, errno, "sem_wait");
}

#ifdef __QNXNTO__
void Sem::WaitTimed(const timespec &absTime, bool *isTimeout) {
	int r;
	
	r = sem_timedwait_monotonic(&sem, &absTime);
	if(r != 0 && errno == ETIMEDOUT) {
		*isTimeout = true;
		return;
	}
	else {
		AssertSysError(r == 0, errno, "pthread_cond_timedwait");
		*isTimeout = false;
	}
}
#endif

void Sem::Post() {
	int r = sem_post(&sem);
	AssertSysError(r == 0, errno, "sem_post");
}

#ifdef __QNXNTO__
Timer::Timer() : event{}, timerId{}, timerCreated{false}, chId{-1} {
	event.sigev_coid = -1;
	
	try {
		unp_assert( (chId = ChannelCreate(0)) != -1 );
		unp_assert( (event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chId, _NTO_SIDE_CHANNEL, 0)) != -1 );
		
		event.sigev_notify = SIGEV_PULSE;
		event.sigev_priority = getprio(0);
		event.sigev_code = _PULSE_CODE_MINAVAIL;
		
		unp_assert( timer_create(CLOCK_MONOTONIC, &event, &timerId) == 0 );
		timerCreated = true;
	}
	catch(...) {
		Release();
		unp_assert(false);
	}
}

Timer::~Timer() {
	Release();
}

void Timer::Release() {
	if(timerCreated) {
		timer_delete(timerId);
		timerCreated = false;
	}
	
	if(event.sigev_coid != -1) {
		if(ConnectDetach(event.sigev_coid) == -1)
			l.AddSysError(errno, "ConnectDetach");
		event.sigev_coid = -1;
	}
	
	if(chId != -1) {
		if(ChannelDestroy(chId) == -1)
			l.AddSysError(errno, "ChannelDestroy");
		chId = -1;
	}
}

void Timer::SetTime(int64_t value_ns, int64_t interval_ns, bool isAbs) {
	itimerspec itime;
	
	itime.it_value.tv_sec = value_ns / NS;
	itime.it_value.tv_nsec = value_ns % NS;
	
	itime.it_interval.tv_sec = interval_ns / NS;
	itime.it_interval.tv_nsec = interval_ns % NS;
	
	AssertSysError( timer_settime(timerId, isAbs ? TIMER_ABSTIME : 0, &itime, NULL) == 0, errno, "timer_settime" );
}

void Timer::Wait() {
	_pulse pulse;
	
	AssertSysError( MsgReceivePulse(chId, &pulse, sizeof(pulse), NULL) == 0, errno, "MsgReceivePulse" );
	
	pthread_testcancel();
}
#endif

static pthread_t errorHandlerTid = 0;
static int errorHandlerSigNumber = 0;

void ThreadInfo::SetErrorHandler(pthread_t tid, int sigNumber) {
	errorHandlerTid = tid;
	errorHandlerSigNumber = sigNumber;
}

pthread_t ThreadInfo::GetErrorHandlerTid() {
	return errorHandlerTid;
}

int ThreadInfo::GetErrorHandlerSigNumber() {
	return errorHandlerSigNumber;
}

ThreadInfo::ThreadInfo(const char *name_, bool quiet_) : ThreadInfo(std::string(name_ ? name_ : ""), quiet_) {
}

ThreadInfo::ThreadInfo(const std::string &name_, bool quiet_) : name(name_), quiet(quiet_) {
	if(!quiet)
		l.AddNote("Thread '%s' begin...", name.c_str());
	pthread_setname_np(pthread_self(), name.c_str());
}

ThreadInfo::~ThreadInfo() {
	if(!quiet)
		l.AddNote("Thread '%s' end", name.c_str());
}

void* ThreadInfo::Run_th(ThreadInfo *thiz) {
	pthread_cleanup_push(ThreadInfo::Delete, thiz);
	
	bool ok;
	
	{
		ThreadCancelDisabler d;
		ok = thiz->Init();
	}
	
	ok = ok && thiz->Process();
	
	if(!ok) {
		// Поток может быть завершён по pthread_cancel().
		// Сюда попадём в случае ошибки.
		if(errorHandlerTid)
			pthread_kill(errorHandlerTid, errorHandlerSigNumber);
	}
	
	pthread_cleanup_pop(1);
	return NULL;
}

void ThreadInfo::Delete(void* thiz) {
	UNP::SetCancelState(PTHREAD_CANCEL_DISABLE); // предыдущий статус м. не возвр-ть
	delete static_cast<ThreadInfo*>(thiz);
}

#ifdef __QNXNTO__
bool SetCpuMask(uint32_t mask) {
	uintptr_t m = mask;
	
	if(ThreadCtl(_NTO_TCTL_RUNMASK, (void*)m) == -1) {
		l.AddSysError(errno, "ThreadCtl _NTO_TCTL_RUNMASK");
		return false;
	}
	
	return true;
}

bool SetCpuMaskInherit(uint32_t runmask) {
	if(!runmask)
		return true;
	
	unsigned *rsizep, rsize, size_tot;
    unsigned *rmaskp, *inheritp;
    unsigned buf[8];
    void *freep;
    
    rsize = RMSK_SIZE(_syspage_ptr->num_cpu);
    
    size_tot = sizeof(*rsizep);
    size_tot += sizeof(*rmaskp) * rsize;
    size_tot += sizeof(*inheritp) * rsize;
    
    if (size_tot <= sizeof(buf)) {
        rsizep = buf;
        freep = NULL;
    }
    else {
		freep = malloc(size_tot);
		if(!freep) {
			l.AddSysError(errno, "malloc");
			return false;
		}
		
		rsizep = static_cast<unsigned*>(freep);
    }
    
    memset(rsizep, 0x00, size_tot);
    
    *rsizep = rsize;
    rmaskp = static_cast<unsigned*>(rsizep + 1);
    inheritp = rmaskp + rsize;
    
    int rsize_bit = rsize * 8;
    for(int i = 0; i < rsize_bit && runmask; ++i) {
        if(runmask & 1) {
           RMSK_SET(i, rmaskp);
           RMSK_SET(i, inheritp);
        }
        runmask >>= 1;
    }
    
    if(ThreadCtl(_NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT, rsizep) == -1) {
        l.AddSysError(errno, "ThreadCtl _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT");
        free(freep);
        return false;
    }
    
    free(freep);
    return true;
}
#endif

}
