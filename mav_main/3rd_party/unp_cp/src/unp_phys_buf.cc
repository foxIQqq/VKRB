/*
 * unp_phys_buf.cc
 *
 *  Created on: 12.04.2016
 *      Author: kudr
 */

#ifdef __QNXNTO__

#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef __PPC__
#include <ppc/cpu.h>
#endif

#include "unp_private.h"

#include "unp_phys_buf.h"

#define F_MODE        ((S_IRUSR | S_IRGRP | S_IROTH) | (S_IWUSR | S_IWGRP | S_IWOTH))
#define MAP_FAILED_U8    static_cast<uint8_t*>(MAP_FAILED)
#define MAP_FAILED_U8_V  static_cast<volatile uint8_t*>(MAP_FAILED)

#define ASSERT_RET(v, prefix) \
	if(!(v)) { \
		l.AddSysError(errno, PREFIX prefix); \
		return false; \
	}

namespace UNP {

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX  "PhysBuf: "

PhysBuf::PhysBuf() : buf(MAP_FAILED_U8), size(0), physAddr(0) {
}

PhysBuf::~PhysBuf() {
	Release();
}

bool PhysBuf::Init(size_t size_, bool noCache) {
	Release();
	
	int r;
	size_t clen;
	int prot = PROT_READ | PROT_WRITE;
	
	if(noCache)
		prot |= PROT_NOCACHE;
	
	buf = static_cast<uint8_t*>(mmap(
				NULL,
				size_,
				prot,
				MAP_ANON | MAP_PHYS /*| MAP_PRIVATE*/,
				NOFD,
				0
			));
	ASSERT_RET(buf != MAP_FAILED_U8, "mmap")
	
	size = size_;
	
	r = mem_offset64(buf, NOFD, size_, &physAddr, &clen);
	ASSERT_RET(r != -1, "mem_offset64")
	
	if(clen != size_) {
		l.AddError(PREFIX "Wrong contiguous block of memory length (%" PRIu64 " instead %" PRIu64 ")", (uint64_t)clen, (uint64_t)size_);
		Release();
		return false;
	}
	
	return true;
}

void PhysBuf::Release() {
	int r;
	
	if(buf != MAP_FAILED_U8) {
		r = munmap(buf, size);
		if(r == -1)
			l.AddSysError(errno, PREFIX "mumnap");
		buf = MAP_FAILED_U8;
	}
	
	size = 0;
	physAddr = 0;
}

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX  "PhysBufShared: "

PhysBufShared::PhysBufShared() : shmFd(-1), buf(MAP_FAILED_U8), size(0), physAddr(0) {
}

PhysBufShared::~PhysBufShared() {
	Release();
}

bool PhysBufShared::Init(const char *name, size_t size_, bool noCache) {
	Release();
	
	if(!name)
		name = "";
	
	int r;
	long pageSize;
	int prot = PROT_READ | PROT_WRITE;
	size_t clen;
	bool created = false;
	
	if(noCache)
		prot |= PROT_NOCACHE;
	
	size = size_;
	
	shmFd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, F_MODE);
	if(shmFd >= 0)
		created = true;
	else if(errno == EEXIST) {
		shmFd = shm_open(name, O_RDWR, F_MODE);
		ASSERT_RET(shmFd >= 0, "shm_open")
		
		struct stat64 st;
		r = fstat64(shmFd, &st);
		ASSERT_RET(r == 0, "fstat64");
		
		if(size > st.st_size) {
			close(shmFd); shmFd = -1;
			shm_unlink(name);
			shmFd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, F_MODE); //-V774
			ASSERT_RET(shmFd >= 0, "shm_open")
			created = true;
		}
	}
	else {
		ASSERT_RET(shmFd >= 0, "shm_open")
	}
	
	if(created) {
		size_t shmSize = size;
		
		/* При использовании shm_ctl() с флагом SHMCTL_PHYS
		 * размер должен быть кратен размеру страницы
		 */
		pageSize = sysconf(_SC_PAGE_SIZE);
		if(pageSize > 0) {
			if(shmSize % pageSize)
				shmSize = (shmSize / pageSize + 1) * pageSize;
		}
		
		/*
		 * Вариант с shm_ctl_special() позволяет отключить кэш
		 * для всех вирт. адресов независимо от PROT_NOCACHE.
		 * Но сейчас он не используется из-за недостаточной документации к флагам special.
		 */
#if 1
		r = shm_ctl(shmFd, SHMCTL_ANON | SHMCTL_PHYS, 0, shmSize);
#else
#if defined __PPC__
		if(noCache)
			r = shm_ctl_special(shmFd, SHMCTL_ANON | SHMCTL_PHYS, 0, shmSize, PPC_SPECIAL_I | PPC_SPECIAL_G);
		else
			r = shm_ctl(shmFd, SHMCTL_ANON | SHMCTL_PHYS, 0, shmSize);
#else
		r = shm_ctl(shmFd, SHMCTL_ANON | SHMCTL_PHYS, 0, shmSize);
#endif
#endif
		ASSERT_RET(r == 0, "shm_ctl*")
	}
	
	buf = static_cast<uint8_t*>(mmap(
			NULL,
			size,
			prot,
			MAP_SHARED,
			shmFd,
			0
		));
	ASSERT_RET(buf != MAP_FAILED_U8, "mmap")
	
	r = mem_offset64(buf, NOFD, size_, &physAddr, &clen);
	ASSERT_RET(r != -1, "mem_offset64")
	
	if(clen != size_) {
		l.AddError(PREFIX "Wrong contiguous block of memory length (%" PRIu64 " instead %" PRIu64 ")", (uint64_t)clen, (uint64_t)size_);
		Release();
		return false;
	}
	
	return true;
}

void PhysBufShared::Release() {
	int r;
	
	if(buf != MAP_FAILED_U8) {
		r = munmap(buf, size);
		if(r == -1)
			l.AddSysError(errno, PREFIX "munmap");
		buf = MAP_FAILED_U8;
	}
	
	if(shmFd >= 0) {
		r = close(shmFd);
		if(r == -1)
			l.AddSysError(errno, PREFIX "close");
		shmFd = -1;
	}
	
	size = 0;
	physAddr = 0;
}

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX  "PhysBufDev: "

PhysBufDev::PhysBufDev() : paddr(0), vaddr(MAP_FAILED_U8_V), size(0), noCache(false) {
}

PhysBufDev::~PhysBufDev() {
	Release();
}

bool PhysBufDev::Init(off64_t paddr_, size_t size_, bool noCache_) {
	if(paddr_ == paddr && size_ == size && noCache_ == noCache)
		return true;
	
	Release();
	
	int prot = PROT_READ | PROT_WRITE;
	
	if(noCache_)
		prot |= PROT_NOCACHE;
	
	vaddr = static_cast<volatile uint8_t*>(mmap_device_memory(NULL, size_, prot, 0, paddr_));
	ASSERT_RET(vaddr != MAP_FAILED, "mmap_device_memory")
	
	paddr = paddr_;
	size = size_;
	noCache = noCache_;
	
	return true;
}

void PhysBufDev::Release() {
	int r;
	
	if(vaddr != MAP_FAILED_U8_V) {
		r = munmap_device_memory(const_cast<uint8_t*>(vaddr), size);
		if(r == -1)
			l.AddSysError(errno, PREFIX "munmap_device_memory");
		vaddr = MAP_FAILED_U8_V;
	}
	
	paddr = 0;
	size = 0;
}

}
#endif
