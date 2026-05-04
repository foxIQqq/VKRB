/*
 * unp_rtdb.cc
 *
 *  Created on: 8 янв. 2024 г.
 *      Author: kudr
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "unp_misc/assert.h"
#include "unp_misc/lock.h"
#include "unp_assert.h"
#include "unp_threads.h"
#include "unp_private.h"

#include "unp_rtdb.h"

#ifdef __QNXNTO__
#define SEP_LOCK // Отдельный файл для блокировок (в QNX блокировка на shmem не поддерживается)
#define FD_LOCK  fdLock
#else
#define FD_LOCK  fd
#endif

#define F_MODE ((S_IRUSR | S_IRGRP | S_IROTH) | (S_IWUSR | S_IWGRP | S_IWOTH))

#define ASSERT(expr, err, prefix)  AssertSysError(expr, err, prefix);

#define ASSERT_T(expr, err, prefix)  if(!(expr)) { \
	l.AddSysError(err, prefix); \
	throw err; \
}

namespace UNP {

class RtdbImpl {
	UNP::CondMutex condMutex;
	unsigned lockCount{};
	bool lockedW{};
	
	int fd{-1};
#ifdef SEP_LOCK
	int fdLock{-1};
#endif
	size_t size{};
	void *data{};
	
	static constexpr int CANCEL_NOT_SET = -1;
	static_assert(PTHREAD_CANCEL_ENABLE != CANCEL_NOT_SET && PTHREAD_CANCEL_DISABLE != CANCEL_NOT_SET, "");
	
	int cancelState{CANCEL_NOT_SET};
	
public:
	RtdbImpl();
	~RtdbImpl();
	
	void Open(const char *path, size_t size);
	const void* LockRead();
	void* LockWrite();
	void Unlock();
	inline void* GetData() { return data; }
	
private:
	void Close();
};

Rtdb::Rtdb() : impl(new RtdbImpl) {
}

Rtdb::~Rtdb() {
	delete impl;
}

void Rtdb::Open(const char *path, size_t size) {
	return impl->Open(path, size);
}

const void* Rtdb::LockRead() {
	return impl->LockRead();
}

void* Rtdb::LockWrite() {
	return impl->LockWrite();
}

void Rtdb::Unlock() {
	return impl->Unlock();
}

void* Rtdb::GetData() {
	return impl->GetData();
}

// *** impl ***

RtdbImpl::RtdbImpl() {
}

RtdbImpl::~RtdbImpl() {
	Close();
	
	if(cancelState != CANCEL_NOT_SET)
		SetCancelState(cancelState);
}

void RtdbImpl::Close() {
	ThreadCancelDisabler d;
	
#ifdef SEP_LOCK
	if(fdLock >= 0) {
		close(fdLock);
		fdLock = -1;
	}
#endif
	
	if(size > 0) {
		munmap(data, size);
		data = NULL;
	}
	
	if(fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void RtdbImpl::Open(const char *path, size_t size_) {
	ThreadCancelDisabler d;
	int r;
	
	Close();
	
	UNP_Assert(path && path[0] && size_ > 0);
	
	fd = shm_open(path, O_RDWR | O_CREAT, F_MODE);
	ASSERT_T(fd >= 0, errno, "shm_open")
	
	r = ftruncate(fd, size_);
	ASSERT_T(r == 0, errno, "ftruncate")
	
	data = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	ASSERT_T(data && data != MAP_FAILED, errno, "mmap")
	
	size = size_;
	
#ifdef SEP_LOCK
	char buf[FILENAME_MAX];
	buf[0] = 0;
	snprintf(buf, sizeof(buf), "/var/run/%s.lock", path);
	fdLock = open(buf, O_RDWR | O_CREAT, F_MODE);
	ASSERT_T(fdLock >= 0, errno, "open .lock")
#endif
}

const void* RtdbImpl::LockRead() {
	SetCancelState(PTHREAD_CANCEL_DISABLE, &cancelState);
	
	condMutex.Lock();
	
	while(lockedW)
		condMutex.Wait();
	
	++lockCount;
//	lockedW = false;
	
	if(lockCount == 1) {
		int r = readw_lock(FD_LOCK);
		ASSERT(r == 0, errno, "readw_lock")
	}
	
	condMutex.Unlock();
	
	return data;
}

void* RtdbImpl::LockWrite() {
	SetCancelState(PTHREAD_CANCEL_DISABLE, &cancelState);
	
	condMutex.Lock();
	
	while(lockCount > 0)
		condMutex.Wait();
	
	lockCount = 1;
	lockedW = true;
	
	int r = writew_lock(FD_LOCK);
	ASSERT(r == 0, errno, "writew_lock")
	
	condMutex.Unlock();
	
	return data;
}

void RtdbImpl::Unlock() {
	condMutex.Lock();
	
	if(lockCount == 1) {
		int r = un_lock(FD_LOCK);
		ASSERT(r == 0, errno, "un_lock")
	}
	
	--lockCount;
	lockedW = false;
	
	condMutex.Unlock();
	condMutex.Trigger();
	
	SetCancelState(cancelState, NULL);
	cancelState = CANCEL_NOT_SET;
}

} // namespace
