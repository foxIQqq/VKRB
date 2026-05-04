/*
 * ip.cc
 *
 *  Created on: 9 мар. 2023 г.
 *      Author: kudr
 */

#ifdef __QNX__
#include <net/netbyte.h>
#else
#include <endian.h>
#endif

#include "unp_misc/ip.h"

namespace UNP {

uint16_t IpCalcChecksum(const void *buf, size_t size) {
	uint32_t sum = 0;
	const uint16_t *p;
	size_t c = size / 2, i;
	uint32_t hi, lo;
	
	for(i = 0, p = static_cast<const uint16_t*>(buf); i < c; ++i, ++p)
		sum += be16toh(*p);
	
	if(size % 2)
		sum += static_cast<const uint8_t*>(buf)[size - 1];
	
	hi = ((sum >> 16) & 0xFFFF);
	while(hi > 0) {
		lo = sum & 0xFFFF;
		sum = hi + lo;
		
		hi = ((sum >> 16) & 0xFFFF);
	}
	
	return ~sum;
}

}
