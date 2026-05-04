/*
 * unp_bpf.cc
 *
 *  Created on: 14 сент. 2023 г.
 *      Author: kudr
 */

#ifdef __QNXNTO__

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/bpf.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "unp_private.h"
#include "unp_sock.h"

#include "unp_bpf.h"

#define PREFIX  "Bpf: "

namespace UNP {

bool BpfLoadFilter(int fd, const char *addr_, unsigned port) {
	in_addr_t addr = inet_addr(addr_);
	
	bpf_insn
		// tcpdump -dd "(dst 239.192.1.57) and (udp dst port 5004)"
		insns_addrSingle[] = {
			{ 0x28, 0, 0, 0x0000000c },
			{ 0x15, 0, 10, 0x00000800 },
			{ 0x20, 0, 0, 0x0000001e },
			{ 0x15, 0, 8, /*0xefc00139*/ be32toh(addr) }, // dst ip
			{ 0x30, 0, 0, 0x00000017 },
			{ 0x15, 0, 6, 0x00000011 },
			{ 0x28, 0, 0, 0x00000014 },
			{ 0x45, 4, 0, 0x00001fff },
			{ 0xb1, 0, 0, 0x0000000e },
			{ 0x48, 0, 0, 0x00000010 },
			{ 0x15, 0, 1, /*0x0000138c*/ port }, // dst port
			{ 0x6, 0, 0, 0x00040000 },
			{ 0x6, 0, 0, 0x00000000 },
		 },
		 // tcpdump -dd "(udp dst port 5004)"
		 insns_addrAny[] = {
			{ 0x28, 0, 0, 0x0000000c },
			{ 0x15, 0, 4, 0x000086dd },
			{ 0x30, 0, 0, 0x00000014 },
			{ 0x15, 0, 11, 0x00000011 },
			{ 0x28, 0, 0, 0x00000038 },
			{ 0x15, 8, 9, /*0x0000138c*/ port }, // dst port
			{ 0x15, 0, 8, 0x00000800 },
			{ 0x30, 0, 0, 0x00000017 },
			{ 0x15, 0, 6, 0x00000011 },
			{ 0x28, 0, 0, 0x00000014 },
			{ 0x45, 4, 0, 0x00001fff },
			{ 0xb1, 0, 0, 0x0000000e },
			{ 0x48, 0, 0, 0x00000010 },
			{ 0x15, 0, 1, /*0x0000138c*/ port }, // dst port
			{ 0x6, 0, 0, 0x00040000 },
			{ 0x6, 0, 0, 0x00000000 },
     	 };
	
	bpf_program program = {0};
	
	if(addr == INADDR_ANY) {
		program.bf_insns = insns_addrAny;
		program.bf_len = sizeof(insns_addrAny) / sizeof(insns_addrAny[0]);
	}
	else {
		program.bf_insns = insns_addrSingle;
		program.bf_len = sizeof(insns_addrSingle) / sizeof(insns_addrSingle[0]);
	}
	
	if(ioctl(fd, BIOCSETF, &program) != 0) {
		l.AddSysError(errno, PREFIX "ioctl BIOCSETF");
		return false;
	}
	
	return true;
}

BpfRecv::BpfRecv() : fd(-1), fdExtra(-1), isOpened(false), rcvBuf(NULL), rcvBufAllocated(0), n(0), offset(0) {
}

BpfRecv::~BpfRecv() {
	Close();
	
	delete []rcvBuf;
}

void BpfRecv::Close() {
	if(fd >= 0) {
		close(fd);
		fd = -1;
	}
	
	if(fdExtra >= 0) {
		close(fdExtra);
		fdExtra = -1;
	}
	
	isOpened = false;
}

bool BpfRecv::Open(const char *ifName, const char *filterAddress, unsigned filterPort, unsigned rcvBufLen, bool immediate) {
	Close();
	
	if(!ifName)
		ifName = "";
	
	if(strlen(ifName) >= sizeof(ifreq::ifr_name)) {
		l.AddError(PREFIX "tpvIfName too big");
		return false;
	}
	
	// fd
	
	const char *path = "/dev/bpf";
	
	fd = open(path, O_RDONLY);
	if(fd < 0) {
		l.AddError(PREFIX "Can't open '%s': %s", path, strerror(errno));
		return false;
	}
	
	if(ioctl(fd, BIOCSBLEN, &rcvBufLen) != 0) {
		l.AddSysError(errno, PREFIX "ioctl BIOCSBLEN");
		return false;
	}
	
	ifreq boundIf{};
	
	strcpy(boundIf.ifr_name, ifName);
	if(ioctl(fd, BIOCSETIF, &boundIf) != 0) {
		l.AddSysError(errno, PREFIX "ioctl BIOCSETIF");
		return false;
	}
	
	{
		unsigned v = immediate;
		if(ioctl(fd, BIOCIMMEDIATE, &v) != 0) {
			l.AddSysError(errno, PREFIX "ioctl BIOCIMMEDIATE");
			return false;
		}
	}
	
	{
		// Блокируем чтение своих пакетов
		unsigned v = 0;
		if(ioctl(fd, BIOCSSEESENT, &v) != 0) {
			l.AddSysError(errno, PREFIX "ioctl BIOCSSEESENT");
			return false;
		}
	}
	
	if(ioctl(fd, BIOCGBLEN, &rcvBufLen) != 0) {
		l.AddSysError(errno, PREFIX "ioctl BIOCGBLEN");
		return false;
	}
	
	if(!BpfLoadFilter(fd, filterAddress, filterPort))
		return false;
	
	ReallocRcvBuf(rcvBufLen);
	
	// fdExtra
	
	fdExtra = UNP::SockUDP_Simple();
	if(fdExtra < 0)
		return false;
	
	in_addr_t inAddr = inet_addr(filterAddress);
	
	if(UNP::Sock_IsMulticastAddress(inAddr)) {
		if(!UNP::Sock_MulticastJoin(fdExtra, inAddr, filterPort, ifName))
			return false;
	}
	else {
		sockaddr_in servaddr{};
		
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inAddr;
		servaddr.sin_port = htons(filterPort);
		
		if(bind(fdExtra, reinterpret_cast<const sockaddr*>(&servaddr), sizeof(servaddr)) != 0) {
			l.AddSysError(errno, PREFIX "bind");
			return false;
		}
	}
	
	isOpened = true;
	return true;
}

void BpfRecv::ReallocRcvBuf(unsigned size) {
	if(size < 1)
		size = 1;
	
	if(size != rcvBufAllocated) {
		delete []rcvBuf; rcvBuf = NULL;
		rcvBuf = new uint8_t[size];
		rcvBufAllocated = size;
	}
	
	n = 0;
	offset = 0;
}

bool BpfRecv::Read() {
	n = read(fd, rcvBuf, rcvBufAllocated);
	
	if(n <= 0) {
		if(n < 0)
			l.AddSysError(errno, PREFIX "read");
		else
			l.AddError(PREFIX "read: return %d", n);
		return false;
	}
	
	offset = 0;
	return true;
}

#pragma pack(1)
struct EthHeader {
	uint8_t dstMac[6];
	uint8_t srcMac[6];
	uint8_t type[2];
};

struct IfHeader {
	EthHeader ethHeader;
	struct ip ipHeader;
	struct udphdr udpHeader;
};
#pragma pack()

bool BpfRecv::Parse(uint8_t **pdatagram, unsigned *plen) {
	if(offset < 0 || offset >= n)
		return false;
	
	struct bpf_hdr* bpfHeader = reinterpret_cast<struct bpf_hdr*>(rcvBuf + offset);
	int offsetNext = offset + BPF_WORDALIGN(bpfHeader->bh_hdrlen + bpfHeader->bh_caplen);
	
	unsigned len = bpfHeader->bh_caplen;
	
	if(len < sizeof(IfHeader))
		return false;
	
	uint8_t *p = rcvBuf + offset + bpfHeader->bh_hdrlen;
	
	*pdatagram = p + sizeof(IfHeader);
	*plen = len - sizeof(IfHeader);
	
	offset = offsetNext;
	
	return true;
}

}
#endif
