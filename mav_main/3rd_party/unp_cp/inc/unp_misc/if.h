/*
 * if.h
 *
 *  Created on: 27 нояб. 2023 г.
 *      Author: kudr
 */

#ifndef INC_UNP_MISC_IF_H_
#define INC_UNP_MISC_IF_H_

#include <stdlib.h>
#include <inttypes.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

namespace UNP {

#pragma pack(1)
struct EthHeader {
	uint8_t dstMac[6];
	uint8_t srcMac[6];
	uint8_t type[2];
};

//! Загловок для передачи по сете UDP-датаграммы
struct IfUdpHeader {
	EthHeader ethHeader;
	struct ip ipHeader;
	struct udphdr udpHeader;
};
#pragma pack()

constexpr size_t MAC_SIZE = 6;

void IfUdpFillHeader(
	IfUdpHeader*,
	const uint8_t srcMac[MAC_SIZE], const char *srcAddress, unsigned srcPort,
	const uint8_t dstMac[MAC_SIZE], const char *dstAddress, unsigned dstPort,
	size_t payloadSize
);

}
#endif /* INC_UNP_MISC_IF_H_ */
