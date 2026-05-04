/*
 * unp_sock.cc
 * 
 *  Created on: 04.12.2013
 *      Author: kudr
 */

#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <net/if.h>
#ifdef __QNXNTO__
#include <net/if_media.h>
#endif
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "unp_private.h"
#include "unp_threads.h"
#include "unp_assert.h"
#include "unp_misc/inet.h"

#include "unp_sock.h"

#define ES  int e = errno;
#define EL  errno = e;

// В QNX есть баг, из-за которого unlink может привести к зависанию io-pkt
#ifndef __QNXNTO__
#define UNLINK
#endif

namespace UNP {

static void SetCloExec(int fd) {
	int flags;
	
	if((flags = fcntl(fd, F_GETFD)) == -1) {
		ES
		l.AddWarning("%s: %s", "fcntl F_GETFD", strerror(e));
		EL
		return;
	}
	
	if(fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		ES
		l.AddWarning("%s: %s", "fcntl F_SETFD", strerror(e));
		EL
		return;
	}
}

static int Sock_Open(int domain, int type) {
	ThreadCancelDisabler dis;
	int fd;
	
	fd = socket(domain, type
#ifdef SOCK_CLOEXEC
			| SOCK_CLOEXEC
#endif
			, 0);
	if(fd < 0) {
		ES
		l.AddSysError(errno, "socket");
		EL
		return fd;
	}
	
#ifndef SOCK_CLOEXEC
	SetCloExec(fd);
#endif
	
	return fd;
}

int SockUDP_Simple() {
	return Sock_Open(AF_INET, SOCK_DGRAM);
}

int SockTCP_Simple() {
	return Sock_Open(AF_INET, SOCK_STREAM);
}

int SockLocalStream_Simple() {
	return Sock_Open(AF_LOCAL, SOCK_STREAM);
}

static void CloseSocket(void* sock_) {
	int sock = (intptr_t)sock_;
	Sock_Close(sock);
}

/*struct SockPath {
	int sock;
	const char *path;
};

static void CloseSocketPath(void* sockPath_) {
	SockPath *sockPath = static_cast<SockPath*>(sockPath_);
	
	Sock_Close(sockPath->sock, sockPath->path);
}*/

void Sock_Close(int sock, const char *localStreamListenPath) {
	ThreadCancelDisabler dis;
	
	close(sock);
	
#ifdef UNLINK
	if(localStreamListenPath && localStreamListenPath[0]) {
		if(unlink(localStreamListenPath) != 0)
			l.AddSysError(errno, "Socket unlink");
	}
#else
	(void)localStreamListenPath;
#endif
}

#ifdef DEBUG_VER
static void SockTCP_LogAddr(struct sockaddr_in &sa_in, const char *prefix) {
	char buf[INET_ADDRSTRLEN];
	buf[0] = 0;
	const char *ret = inet_ntop(AF_INET, &sa_in.sin_addr, buf, sizeof(buf));
	l.AddNote("%s%s:%u", prefix ? prefix : "", ret ? ret : "UNK", unsigned(ntohs(sa_in.sin_port)));
}
#endif

static bool Connect(int s, struct sockaddr *addr, socklen_t len, int timeout_ms) {
	const char *prefix = "Connection error";
	
	if(timeout_ms >= 0) {
		class NonBlock {
			int s;
			int oflagsOrig;
			
		public:
			NonBlock(int s_) : s (s_) {
				int oflags;
				
				oflagsOrig = oflags = fcntl(s, F_GETFL);
				oflags |= O_NONBLOCK;
				fcntl(s, F_SETFL, oflags);
			}
			
			~NonBlock() {
				fcntl(s, F_SETFL, oflagsOrig);
			}
		} nonBlock(s);
		
		int r;
		
		r = connect(s, addr, len);
		if(r != 0 && errno != EINPROGRESS) {
			ES
			l.AddError("%s: %s", prefix, strerror(errno));
			EL
			return false;
		}
		
		if(r != 0) {
			pollfd fds = {0};
			
			fds.fd = s;
			fds.events = POLLOUT;
			
			r = poll(&fds, 1, timeout_ms);
			if(r == 0) {
				l.AddError("%s: %s", prefix, "Timed out");
				errno = ETIMEDOUT;
				return false;
			}
			else if(r < 0) {
				ES
				l.AddError("%s: %s: %s", prefix, "poll", strerror(errno));
				EL
				return false;
			}
			
			{
				int err = 0;
				socklen_t len = sizeof(err);
				
				r = getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len);
				if(r != 0) {
					ES
					l.AddError("%s: %s: %s", prefix, "getsockopt SO_ERROR", strerror(errno));
					EL
					return false;
				}
				
				if(err != 0) {
					l.AddError("%s: %s", prefix, strerror(err));
					errno = err;
					return false;
				}
			}
		}
	}
	else {
		if(connect(s, addr, len) != 0) {
			ES
			l.AddError("%s: %s", prefix, strerror(errno));
			EL
			return false;
		}
	}
	
	return true;
}

int SockTCP_Connect(const char *address, unsigned port, int timeout_ms) {
	int sock = SockTCP_Simple();
	if(sock < 0)
		return sock;
	
	sockaddr_in servAddr = {};
	socklen_t servLen = sizeof(servAddr);
	bool ok;
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(address);
	servAddr.sin_port = htons(port);
	
	pthread_cleanup_push(CloseSocket, (void*)(intptr_t)sock);
	ok = Connect(sock, reinterpret_cast<struct sockaddr*>(&servAddr), servLen, timeout_ms);
	pthread_cleanup_pop(0);
	
	if(ok)
		return sock;
	else {
		ES
		Sock_Close(sock);
		EL
		return -1;
	}
}

static Mutex& GetAddrInfoMutex() {
	static_assert(__cplusplus >= 201103L, ""); // C++11
	static Mutex m;
	return m;
}

static void GetAddrInfoClean(void *info_) {
	addrinfo **info = static_cast<struct addrinfo**>(info_);
	
	if(*info)
		freeaddrinfo(*info);
	
	GetAddrInfoMutex().Unlock();
}

// pinfo not NULL
static bool SockTCP_GetAddrInfo(const char *nodeName, const char *servName, struct sockaddr_in *pinfo) {
	// Функция getaddrinfo() не безопасна в потоках и является точкой отмены
	
	addrinfo *result = NULL;
	addrinfo hints;
	int r;
	int e;
	
	if(!nodeName)
		nodeName = "";
	if(!servName)
		servName = "";
	
	bzero(pinfo, sizeof(*pinfo));
	pinfo->sin_family = AF_INET;
	
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	GetAddrInfoMutex().Lock();
	pthread_cleanup_push(GetAddrInfoClean, &result);
	
	r = getaddrinfo(nodeName, servName, &hints, &result);
	e = errno;
	
	pthread_cleanup_pop(1);
	
	if(r != 0) {
		l.AddError("%s: '%s' error (%d - '%s')", __func__, "getaddrinfo", r, gai_strerror(r));
		errno = e;
		return false;
	}
	
	pinfo->sin_addr.s_addr = (reinterpret_cast<struct sockaddr_in*>(result->ai_addr))->sin_addr.s_addr;
	pinfo->sin_port = (reinterpret_cast<struct sockaddr_in*>(result->ai_addr))->sin_port;
	
	return true;
}

int SockTCP_Connect_names(const char *nodeName, const char *servName, int connectTimeout_ms) {
	int sock = SockTCP_Simple();
	if(sock < 0)
		return sock;
	
	sockaddr_in servAddr;
	socklen_t servLen = sizeof(servAddr);
	bool ok;
	
	pthread_cleanup_push(CloseSocket, (void*)(intptr_t)sock);
	ok = SockTCP_GetAddrInfo(nodeName, servName, &servAddr) && Connect(sock, reinterpret_cast<struct sockaddr*>(&servAddr), servLen, connectTimeout_ms);
	pthread_cleanup_pop(0);
	
	if(ok)
		return sock;
	else {
		ES
		Sock_Close(sock);
		EL
		return -1;
	}
}

int SockTCP_Listen(const char *address, unsigned port, int listenQueueLen) {
	ThreadCancelDisabler dis;
	int sock;
	const int one = 1;
	bool any = !address || !address[0];
	sockaddr_in servAddr = {};
	
//	l.AddNote("Attempt to listen '%s:%u'", any ? "<ANY>" : address, unsigned(port));
	
	sock = SockTCP_Simple();
	if(sock < 0)
		return sock;
	
	try {
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
			throw "setsockopt SO_REUSEADDR";
		
		if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)) != 0)
			throw "setsockopt SO_KEEPALIVE";
		
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = any ? INADDR_ANY : inet_addr(address);
		servAddr.sin_port = htons(port);
		
		if(bind(sock, reinterpret_cast<struct sockaddr*>(&servAddr), sizeof(servAddr)) != 0)
			throw "bind";
		
		if(listen(sock, listenQueueLen) != 0)
			throw "listen";
	}
	catch(const char *s) {
		ES
		l.AddSysError(errno, s);
		Sock_Close(sock);
		EL
		return -1;
	}
	
//	l.AddNote("Listen begin");
	return sock;
}

int SockTCP_Accept(int sockListen) {
	int sockConn;
	sockaddr_in cliAddr;
	socklen_t cliLen = sizeof(cliAddr);
	
	l.AddDebug("Accept");
	
	sockConn = accept(sockListen, reinterpret_cast<struct sockaddr*>(&cliAddr), &cliLen);
	if(sockConn < 0) {
		ES
		l.AddSysError(errno, "accept");
		EL
		return sockConn;
	}
	
	SetCloExec(sockConn);
#ifdef DEBUG_VER
	SockTCP_LogAddr(cliAddr, "Connection from: ");
#endif
	
	return sockConn;
}

bool Sock_SetTimeouts(int sock, int timeout_msec, unsigned target) {
	if(timeout_msec < 0)
		return true;
	
	struct timeval tval;
	div_t d = div(timeout_msec, 1000);
	
	tval.tv_sec = d.quot;
	tval.tv_usec = d.rem * 1000;
	
	if(target & (1 << 0)) {
		if(!Sock_SetOpt(sock, SOL_SOCKET, "SO_SNDTIMEO", SO_SNDTIMEO, &tval, sizeof(tval)))
			return false;
	}
	
	if(target & (1 << 1)) {
		if(!Sock_SetOpt(sock, SOL_SOCKET, "SO_RCVTIMEO", SO_RCVTIMEO, &tval, sizeof(tval)))
			return false;
	}
	
	return true;
}

bool Sock_SetOpt(int sock, int level, const char *optNameS, int optName, const void * optVal, socklen_t optLen) {
	if(setsockopt(sock, level, optName, optVal, optLen) != 0) {
		ES
		l.AddError("%s %s: %s (%d)", "setsockopt", optNameS, strerror(e), e);
		EL
		return false;
	}
	return true;
}

ssize_t Sock_Read(int sock, void *buf, size_t n, bool logEof, bool logTimeout, bool logConnReset) {
	ssize_t r = recv(sock, buf, n, 0);
	
	/*{
		fflush(stderr);
		printf("Received %d bytes:", int(r));
		for(int i = 0; i < r; ++i) {
			printf(" %02X", ((uint8_t*)buf)[i]);
		}
		printf("\n");
		fflush(stdout);
	}*/
	
	if(r < 0) {
		ES
		bool skip = (e == EAGAIN && !logTimeout) || (e == ECONNRESET && !logConnReset);
		if(!skip)
			l.AddSysError(e, "Socket recv");
		EL
	}
	else if(r == 0) {
		if(logEof)
			l.AddError("Socket unexpected EOF");
		errno = ENODATA;
	}
	
	return r;
}

bool Sock_ReadN(int sock, void *buf, size_t n, bool logEof, bool logTimeout, bool logConnReset) {
	ssize_t r = recv(sock, buf, n, MSG_WAITALL);
	
	/*{
		fflush(stderr);
		printf("Received %d bytes:", int(r));
		for(int i = 0; i < r; ++i) {
			printf(" %02X", ((uint8_t*)buf)[i]);
		}
		printf("\n");
		fflush(stdout);
	}*/
	
	if(r == ssize_t(n))
		return true;
	else {
		if(r < 0) {
			ES
			bool skip = (e == EAGAIN && !logTimeout) || (e == ECONNRESET && !logConnReset);
			if(!skip)
				l.AddSysError(e, "Socket recv");
			EL
		}
		else if(r > 0) {
			l.AddError("Read from socket %" PRId64 " bytes instead %" PRIu64, int64_t(r), uint64_t(n));
			errno = EBADE;
		}
		else {
			if(logEof)
				l.AddError("Socket unexpected EOF");
			errno = ENODATA;
		}
		return false;
	}
}

bool Sock_ReadNV(int sock, iovec *iov, int iovCnt, size_t totalSize, bool logEof, bool logTimeout) {
	msghdr msgH{};
	
	msgH.msg_iov = iov;
	msgH.msg_iovlen = iovCnt;
	
	ssize_t r = recvmsg(sock, &msgH, MSG_WAITALL);
	
	if(r == ssize_t(totalSize))
		return true;
	else {
		if(r < 0) {
			ES
			if(e != EAGAIN || logTimeout)
				l.AddSysError(e, "Socket recvmsg");
			EL
		}
		else if(r > 0) {
			l.AddError("Read from socket %" PRId64 " bytes instead %" PRIu64, int64_t(r), uint64_t(totalSize));
			errno = EBADE;
		}
		else {
			if(logEof)
				l.AddError("Socket unexpected EOF");
			errno = ENODATA;
		}
		return false;
	}
}

bool Sock_WriteN(int sock, const void *buf, size_t n) {
	/*{
		fflush(stderr);
		printf("Write %d bytes:", int(n));
		for(size_t i = 0; i < n; ++i) {
			printf(" %02X", ((const uint8_t*)buf)[i]);
		}
		printf("\n");
		fflush(stdout);
	}*/
	
	ssize_t r = send(sock, buf, n, MSG_NOSIGNAL);
	
	if(r == ssize_t(n))
		return true;
	else {
		if(r < 0) {
			ES
			l.AddSysError(errno, "Socket send");
			EL
		}
		else {
			l.AddError("Write to socket %" PRId64 " bytes instead %" PRIu64, int64_t(r), uint64_t(n));
			errno = r == 0 ? ENODATA : EBADE;
		}
		return false;
	}
}

bool Sock_WriteNV(int sock, const iovec *iov, int iovCnt, size_t totalSize) {
	msghdr msgH{};
	
	msgH.msg_iov = const_cast<iovec*>(iov);
	msgH.msg_iovlen = iovCnt;
	
	ssize_t r = sendmsg(sock, &msgH, MSG_NOSIGNAL);
	
	if(r == ssize_t(totalSize))
		return true;
	else {
		if(r < 0) {
			ES
			l.AddSysError(errno, "Socket sendmsg");
			EL
		}
		else {
			l.AddError("Write to socket %" PRId64 " bytes instead %" PRIu64, int64_t(r), uint64_t(totalSize));
			errno = r == 0 ? ENODATA : EBADE;
		}
		return false;
	}
}

int SockLocalStream_Connect(const char *path, int timeout_ms) {
	if(!path)
		path = "";
	
	if(!path[0]) {
		l.AddError("Path is empty");
		errno = EINVAL;
		return -1;
	}
	
	sockaddr_un servAddr = {};
	socklen_t servLen = sizeof(servAddr);
	
	if(strlen(path) >= sizeof(servAddr.sun_path)) {
		l.AddError("Path too big");
		errno = EINVAL;
		return -1;
	}
	
	int sock = SockLocalStream_Simple();
	if(sock < 0)
		return sock;
	
	servAddr.sun_family = AF_LOCAL;
	strcpy(servAddr.sun_path, path);
	
	bool ok;
	
	pthread_cleanup_push(CloseSocket, (void*)(intptr_t)sock);
	ok = Connect(sock, reinterpret_cast<struct sockaddr*>(&servAddr), servLen, timeout_ms);
	pthread_cleanup_pop(0);
	
	if(ok)
		return sock;
	else {
		ES
		Sock_Close(sock, NULL);
		EL
		return -1;
	}
}

int SockLocalStream_Listen(const char *path, int listenQueueLen) {
	ThreadCancelDisabler dis;
	int sock;
//	const int one = 1;
	sockaddr_un servAddr = {};
	bool needUnlink = false;
	
	if(!path)
		path = "";
	
//	l.AddNote("Attempt to listen '%s'", path);
	
	if(!path[0]) {
		l.AddError("Path is empty");
		errno = EINVAL;
		return -1;
	}
	
	if(strlen(path) >= sizeof(servAddr.sun_path)) {
		l.AddError("Path too big");
		errno = EINVAL;
		return -1;
	}
	
	sock = SockLocalStream_Simple();
	if(sock < 0)
		return sock;
	
#ifdef UNLINK
	unlink(path);
#endif
	
	try {
		/*if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
			throw "setsockopt SO_REUSEADDR";
		
		if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)) != 0)
			throw "setsockopt SO_KEEPALIVE";*/
		
		servAddr.sun_family = AF_LOCAL;
		strcpy(servAddr.sun_path, path);
		
		if(bind(sock, reinterpret_cast<struct sockaddr*>(&servAddr), sizeof(servAddr)) != 0)
			throw "bind";
		
		needUnlink = true;
		
		if(chmod(path, 0777) != 0)
			throw "chmod";
		
		if(listen(sock, listenQueueLen) != 0)
			throw "listen";
	}
	catch(const char *s) {
		ES
		l.AddSysError(errno, s);
		Sock_Close(sock, needUnlink ? path : NULL);
		EL
		return -1;
	}
	
//	l.AddNote("Listen begin");
	return sock;
}

int SockLocalStream_Accept(int sockListen) {
	int sockConn;
	sockaddr_un cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	
	l.AddDebug("Accept");
	
	sockConn = accept(sockListen, reinterpret_cast<struct sockaddr*>(&cliaddr), &clilen);
	if(sockConn < 0) {
		ES
		l.AddSysError(errno, "accept");
		EL
		return sockConn;
	}
	
	SetCloExec(sockConn);
	l.AddDebug("Incoming connection");
	return sockConn;
}

bool Sock_GetIfAddress(in_addr_t *pres, const char *iface, bool cycle) {
	if(iface == NULL) {
		*pres = htonl(INADDR_ANY);
		l.AddNote("Iface address: %s", inet_addr_ntop(*pres).c_str());
		return true;
	}
	
	if(isdigit(iface[0])) {
		*pres = inet_addr(iface);
		l.AddNote("Iface address: %s", inet_addr_ntop(*pres).c_str());
		return true;
	}
	
	ifreq ifreq = {};
	
	strncpy(ifreq.ifr_name, iface, IFNAMSIZ-1);
	
	l.AddNote("Determining address of iface '%s'...", iface);
	
	do {
		try {
			int sock;
			int r;
			
			// socket
			{
				UNP::ThreadCancelDisabler d;
				sock = socket(AF_INET, SOCK_DGRAM, 0);
				if(sock < 0)
					throw "socket";
			}
			
			// ioctl
			pthread_cleanup_push(CloseSocket, (void*)(intptr_t)sock);
			r = ioctl(sock, SIOCGIFADDR, &ifreq);
			if(r < 0)
				throw "ioctl SIOCGIFADDR";
			pthread_cleanup_pop(1);
			
			// ok
			*pres = reinterpret_cast<sockaddr_in*>(&ifreq.ifr_addr)->sin_addr.s_addr;
			l.AddNote("Iface address of '%s': %s", iface, inet_addr_ntop(*pres).c_str());
			return true;
		}
		catch(const char *s) {
			if(!cycle) {
				ES
				l.AddSysError(errno, s);
				EL
			}
		}
		
		// error
		if(cycle)
			usleep(100000);
	} while(cycle);
	
	return false;
}

bool Sock_IsMulticastAddress(in_addr_t address) {
	// Note: We return False for addresses in the range 224.0.0.0
	// through 224.0.0.255, because these are non-routable
	// Note: IPv4-specific #####
	address = ntohl(address);
	return address > 0xE00000FF && address <= 0xEFFFFFFF;
}

bool Sock_IsMulticastAddress(const char *address) {
	return Sock_IsMulticastAddress(inet_addr(address));
}

bool Sock_MulticastSetIface(int sock, const char *iface) {
	in_addr inaddr = {};
	
	if(iface != NULL) {
		if(isdigit(iface[0])) {
			inaddr.s_addr = inet_addr(iface);
		}
		else {
			ifreq ifreq = {};
			
			strncpy(ifreq.ifr_name, iface, IFNAMSIZ-1);
			
			if(ioctl(sock, SIOCGIFADDR, &ifreq) < 0) {
				ES
				l.AddSysError(errno, "ioctl SIOCGIFADDR");
				EL
				return false;
			}
			
			memcpy(&inaddr, &reinterpret_cast<struct sockaddr_in*>(&ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
		}
	}
	else
		inaddr.s_addr = htonl(INADDR_ANY);	/* remove prev. set default */
	
	if(!Sock_SetOpt(sock, IPPROTO_IP, "IP_MULTICAST_IF", IP_MULTICAST_IF, &inaddr, sizeof(struct in_addr)))
		return false;
	
	return true;
}

int SockUDP_Connect(const char *addr, unsigned port, const char *iface, bool loopEnable) {
	UNP::ThreadCancelDisabler d;
	
	int sock = SockUDP_Simple();
	if(sock < 0)
		return sock;
	
	if(!addr)
		addr = "";
	if(!iface)
		iface = "";
	
	int r;
	struct sockaddr_in servaddr = {};
	in_addr_t in_addr = inet_addr(addr);
	bool isMcast = Sock_IsMulticastAddress(in_addr);
	
	try {
		if(isMcast) {
			{
				u_char flag = loopEnable ? 1 : 0;
				Sock_SetOpt(sock, IPPROTO_IP, "IP_MULTICAST_LOOP", IP_MULTICAST_LOOP, &flag, sizeof(flag));
			}
			
			if(!Sock_MulticastSetIface(sock, iface))
				throw 0;
		}
		else {
			{
				const int v = 1;
				Sock_SetOpt(sock, SOL_SOCKET, "SO_BROADCAST", SO_BROADCAST, &v, sizeof(v));
			}
			
#if 0
			// Here we bind our socket to certain interface to ensure not to spam  in real LAN.
			// SU rights are required. That is why we comment this out
			r = no_mcast_set_if(fd, ifName);
			ASSERT_RET(r == 0, "no_mcast_set_if (sudo required)");
#endif
		}
		
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = in_addr;
		servaddr.sin_port = htons(port);
		
		r = connect(sock, reinterpret_cast<const struct sockaddr*>(&servaddr), sizeof(servaddr));
		if(r != 0) {
			ES
			l.AddSysError(errno, "connect");
			EL
			throw 0;
		}
	}
	catch(...) {
		ES
		Sock_Close(sock);
		EL
		return -1;
	}
	
	return sock;
}

bool Sock_SetTtl(int sock, int ttl) {
	return Sock_SetOpt(sock, IPPROTO_IP, "IP_TTL", IP_TTL, &ttl, sizeof(ttl));
}

bool Sock_SetMulticastTtl(int sock, int val) {
	u_char ttl = val;
	return Sock_SetOpt(sock, IPPROTO_IP, "IP_MULTICAST_TTL", IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

// Join по адресу
static int mcast_join(int sockfd, in_addr_t grpAddr, in_addr_t ifAddr) {
	struct ip_mreq imr = {};
	
	imr.imr_multiaddr.s_addr = grpAddr;
	imr.imr_interface.s_addr = ifAddr;
	
	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&imr, sizeof(imr)) != 0) {
		return -1;
	}
	
	return 0;
}

// Из Стивенса
#ifdef MCAST_JOIN_GROUP
static int family_to_level(int family) {
	switch (family) {
	case AF_INET:
		return IPPROTO_IP;
#ifdef	IPV6
	case AF_INET6:
		return IPPROTO_IPV6;
#endif
	default:
		return -1;
	}
}
#endif
//
static int
mcast_join(int sockfd, const struct sockaddr *grp, socklen_t grplen,
		   const char *ifname, u_int ifindex)
{
#ifdef MCAST_JOIN_GROUP
	struct group_req req;
	if (ifindex > 0) {
		req.gr_interface = ifindex;
	} else if (ifname != NULL) {
		if ( (req.gr_interface = if_nametoindex(ifname)) == 0) {
			errno = ENXIO;	// i/f name not found
			return(-1);
		}
	} else
		req.gr_interface = 0;
	if (grplen > sizeof(req.gr_group)) {
		errno = EINVAL;
		return -1;
	}
	memcpy(&req.gr_group, grp, grplen);
	return (setsockopt(sockfd, family_to_level(grp->sa_family),
			MCAST_JOIN_GROUP, &req, sizeof(req)));
#else
// end mcast_join1
	
// include mcast_join2
	(void)grplen;
	
	switch (grp->sa_family) {
	case AF_INET: {
		struct ip_mreq		mreq;
		struct ifreq		ifreq;

		memcpy(&mreq.imr_multiaddr,
			   &((const struct sockaddr_in *) grp)->sin_addr,
			   sizeof(struct in_addr));

		if (ifindex > 0) {
			if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
				errno = ENXIO;	// i/f index not found
				return(-1);
			}
			goto doioctl;
		} else if (ifname != NULL) {
			strncpy(ifreq.ifr_name, ifname, IFNAMSIZ-1);
doioctl:
			if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
				return(-1);
			memcpy(&mreq.imr_interface,
				   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
				   sizeof(struct in_addr));
		} else
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		
		return(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
						  &mreq, sizeof(mreq)));
	}
// end mcast_join2

// include mcast_join3
#ifdef	IPV6
#ifndef	IPV6_JOIN_GROUP		// APIv0 compatibility
#define	IPV6_JOIN_GROUP		IPV6_ADD_MEMBERSHIP
#endif
	case AF_INET6: {
		struct ipv6_mreq	mreq6;

		memcpy(&mreq6.ipv6mr_multiaddr,
			   &((const struct sockaddr_in6 *) grp)->sin6_addr,
			   sizeof(struct in6_addr));

		if (ifindex > 0) {
			mreq6.ipv6mr_interface = ifindex;
		} else if (ifname != NULL) {
			if ( (mreq6.ipv6mr_interface = if_nametoindex(ifname)) == 0) {
				errno = ENXIO;	// i/f name not found
				return(-1);
			}
		} else
			mreq6.ipv6mr_interface = 0;

		return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
						  &mreq6, sizeof(mreq6)));
	}
#endif

	default:
		errno = EAFNOSUPPORT;
		return(-1);
	}
#endif
}
// end mcast_join3

int SockUDP_Listen(const char *address, unsigned port, const char *iface) {
	if(!address)
		address = "";
	
	ThreadCancelDisabler dis;
	int sock;
	int one = 1;
	bool any = !address[0];
	in_addr_t in_addr = any ? INADDR_ANY : inet_addr(address);
	sockaddr_in servAddr = {0};
	bool isMcast = Sock_IsMulticastAddress(in_addr);
	
//	l.AddNote("Attempt to listen '%s:%u'", any ? "<ANY>" : address, port);
	
	sock = SockUDP_Simple();
	if(sock < 0)
		return sock;
	
	try {
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
			throw "setsockopt SO_REUSEADDR";
		
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) != 0)
			throw "setsockopt SO_REUSEPORT";
		
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = in_addr;
		servAddr.sin_port = htons(port);
		
		if(bind(sock, reinterpret_cast<struct sockaddr*>(&servAddr), sizeof(servAddr)) != 0)
			throw "bind";
		
		if(isMcast) {
			if(!Sock_MulticastJoin(sock, in_addr, port, iface))
				throw "";
		}
	}
	catch(const char *s) {
		ES
		if(s[0])
			l.AddSysError(errno, s);
		Sock_Close(sock);
		EL
		return -1;
	}
	
//	l.AddNote("Listen begin");
	return sock;
}

bool Sock_MulticastJoin(int sock, in_addr_t groupAddress, unsigned groupPort, const char *iface) {
	if(!iface)
		iface = "";
	
	if(isdigit(iface[0])) {
		if(mcast_join(sock, groupAddress, inet_addr(iface)) != 0) {
			ES
			l.AddSysError(errno, "mcast_join (by ifAddr)");
			EL
			return false;
		}
	}
	else {
		struct sockaddr_in sa = {};
		
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = groupAddress;
		sa.sin_port = htons(groupPort);
		
		if(mcast_join(sock, reinterpret_cast<const struct sockaddr*>(&sa), sizeof(sa), iface, 0) != 0) {
			ES
			l.AddSysError(errno, "mcast_join");
			EL
			return false;
		}
	}
	
#ifdef IP_MULTICAST_ALL
	{
		int multicastAll = 0;
		Sock_SetOpt(sock, IPPROTO_IP, "IP_MULTICAST_ALL", IP_MULTICAST_ALL, &multicastAll, sizeof(multicastAll));
	}
#endif
	
	return true;
}

#if 0
static int no_mcast_set_if(int fd, const char *ifname) {
    if(!ifname || *ifname == 0)
        return 0;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFINDEX, &ifr);

    return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,  &ifr, sizeof(struct ifreq));
}
#endif

#ifdef __QNXNTO__
int Sock_GetIfLinkStatus(int sock, const char *name) {
	UNP_Assert(sock >= 0);
	
	if(!name)
		name = "";
	
	ifmediareq ifmr = {0};
	int r;
	
	strlcpy(ifmr.ifm_name, name, sizeof(ifmr.ifm_name));
	r = ioctl(sock, SIOCGIFMEDIA, &ifmr);
	if(r < 0) {
		ES
		l.AddSysError(errno, "%s: ioctl SIOCGIFMEDIA", name);
		EL
		return -1;
	}
	else {
		if((ifmr.ifm_status & (IFM_AVALID|IFM_ACTIVE)) == (IFM_AVALID|IFM_ACTIVE))
			return 1;
		else
			return 0;
	}
}
#endif

}
