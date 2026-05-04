/*
 * string.cc
 *
 *  Created on: 22.03.2019
 *      Author: kudr
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "unp_misc/assert.h"

#include "unp_misc/string.h"

namespace UNP {

uint32_t string_bits_to_mask32(const char *s_, char sep_) {
	if(!s_)
		s_ = "";
	
	const uint32_t one = 1;
	unsigned long v;
	uint32_t mask = 0;
	char *s, *sep, *p;
	
	s = strdup(s_);
	unp_assert(s);
	
	p = s;
	
	for(;;) {
		sep = strchr(p, sep_); //-V575
		if(sep)
			*sep = 0;
		v = strtoul(p, 0, 0);
		mask |= one << v;
		
		if(!sep)
			break;
		
		p = sep + 1;
	}
	
	free(s);
	
	return mask;
}

std::string string_bits_from_mask32(uint32_t mask, char sep) {
	std:: string s;
	char buf[64];
	
	buf[0] = 0;
	
	for(int bit = 0; mask; mask >>= 1, ++bit) if(mask & 1) {
		snprintf(buf, sizeof(buf), "%d", bit);
		if(!s.empty())
			s += sep;
		s += buf;
	}
	
	return s;
}

}
