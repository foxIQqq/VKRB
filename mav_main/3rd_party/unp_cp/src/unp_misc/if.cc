/*
 * if.cc
 *
 *  Created on: 27 нояб. 2023 г.
 *      Author: kudr
 */

#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>

#ifdef __QNX__
#include <net/netbyte.h>
#else
#include <endian.h>
#endif

#include "unp_misc/ip.h"

#include "unp_misc/if.h"

namespace UNP {

void IfUdpFillHeader(
	IfUdpHeader *ifHeader,
	const uint8_t srcMac[MAC_SIZE], const char *srcAddress, unsigned srcPort,
	const uint8_t dstMac[MAC_SIZE], const char *dstAddress, unsigned dstPort,
	size_t payloadSize
)
{
	// eth
	{
		EthHeader &h = ifHeader->ethHeader;
		
		memcpy(h.dstMac, dstMac, MAC_SIZE * sizeof(dstMac[0]));
		memcpy(h.srcMac, srcMac, MAC_SIZE * sizeof(srcMac[0]));
		h.type[0] = 0x08;
		h.type[1] = 0x00;
	}
	
	// ip
	{
		struct ip &h = ifHeader->ipHeader;
		uint32_t ipLen, ipSum;
		
		ipLen = sizeof(IfUdpHeader) - offsetof(IfUdpHeader, ipHeader) + payloadSize;
		
		h.ip_v = 4;
		h.ip_hl = 5;
		h.ip_len = htobe16(ipLen);
		h.ip_ttl = 8; // На маленькие значения ругается Wireshark
		h.ip_p = IPPROTO_UDP;
		h.ip_src.s_addr = inet_addr(srcAddress);
		h.ip_dst.s_addr = inet_addr(dstAddress);
		h.ip_off = htobe16(IP_DF);
		
		ipSum = IpCalcChecksum(&h, sizeof(h));
		
		h.ip_sum = htobe16(ipSum);
	}
	
	// udp
	{
		struct udphdr &h = ifHeader->udpHeader;
		uint32_t udpLen;
		
		udpLen = sizeof(IfUdpHeader) - offsetof(IfUdpHeader, udpHeader) + payloadSize;
		
		h.uh_dport = htobe16(dstPort);
		h.uh_sport = htobe16(srcPort);
		h.uh_ulen = htobe16(udpLen);
		h.uh_sum = htobe16(0);
	}
}

}
