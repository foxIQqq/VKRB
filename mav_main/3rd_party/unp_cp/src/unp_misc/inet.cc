#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/socket.h>

#include "unp_misc/inet.h"

namespace UNP {

#define ADDR_VALID(ptr) ((ptr)->ifa_addr && (ptr)->ifa_netmask && (ptr)->ifa_addr->sa_family == AF_INET)

IfiInfo::IfiInfo() : flags(0), addr(0), netMask(0), brdAddr(0) {
}

int get_ifi_info(std::list<IfiInfo> &dst) {
	int r;
	struct ifaddrs *ifp = NULL, *ptr;
	IfiInfo item;
	
	dst.clear();
	
	r = getifaddrs(&ifp);
	if(r != 0)
		return r;
	
	for(ptr = ifp; ptr; ptr = ptr->ifa_next) if(ADDR_VALID(ptr)) {
		item.flags = ptr->ifa_flags;
		item.addr = ((sockaddr_in*)ptr->ifa_addr)->sin_addr.s_addr;
		item.netMask = ((sockaddr_in*)ptr->ifa_netmask)->sin_addr.s_addr;
		item.brdAddr = item.addr | ~item.netMask;
		
		dst.push_back(item);
	}
	
	freeifaddrs(ifp);
	return 0;
}

std::string inet_addr_ntop(in_addr_t addr) {
	union {
		in_addr_t a;
		uint8_t b[4];
	} a;
	char buf[17];
	int r;
	
	a.a = addr;
	buf[0] = 0;
	r = snprintf(buf, sizeof(buf), "%u.%u.%u.%u", (unsigned)a.b[0], (unsigned)a.b[1], (unsigned)a.b[2], (unsigned)a.b[3]);
	if(r < 0)
		return "";
	
	return buf;
}

}
