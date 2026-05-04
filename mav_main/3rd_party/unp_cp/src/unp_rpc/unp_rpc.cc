/*
 * unp_rpc.cc
 * 
 * RPC_Message и др.
 *
 *  Created on: 31.01.2014
 *      Author: kudr
 */

#include <string.h>
#include <malloc.h>

#ifdef __QNX__
#include <net/netbyte.h>
#else
#include <endian.h>
#endif

#include "unp_assert.h"
#include "unp_misc/malloc.h"

#include "unp_rpc_private.h"

#pragma pack(1)
struct MsgBinHeader {
	uint8_t prefix;
	uint8_t type; // метод или сигнал
	uint16_t cmd;
	uint32_t dataLen;
};
#pragma pack()

static const uint8_t MSG_BIN_HEADER_PREFIX = 0xB0; // д.б. не ascii

static const uint8_t MSG_BIN_HEADER_TYPE_METHOD = '*';
static const uint8_t MSG_BIN_HEADER_TYPE_SIGNAL = '.';

namespace UNP {

RPC_Args::RPC_Args() : parseError(false) {
	message = new RPC_Message;
}

RPC_Args::~RPC_Args() {
	delete message;
}

void RPC_Args::Clear() {
	parseError = false;
	message->Clear();
}

#define ARGS_SHIFT_LEFT(type) \
	RPC_Args& RPC_Args::operator << (type v) { \
		message->packer->PutVar(v); \
		return *this; \
	}

ARGS_SHIFT_LEFT(int32_t)
ARGS_SHIFT_LEFT(uint32_t)
ARGS_SHIFT_LEFT(int64_t)
ARGS_SHIFT_LEFT(uint64_t)
ARGS_SHIFT_LEFT(double)

RPC_Args& RPC_Args::operator << (const std::string &v) {
	message->packer->PutVar(v.c_str(), v.length());
	return *this;
}

RPC_Args& RPC_Args::operator << (const char *v) {
	if(!v)
		v = "";
	
	message->packer->PutVar(v, strlen(v));
	return *this;
}

#define ARGS_SHIFT_RIGHT(type, vDefault) \
	RPC_Args& RPC_Args::operator >> (type &v) { \
		if(!parseError) { \
			try { \
				message->packer->GetVar_e(v); \
			} \
			catch(...) { \
				parseError = true; \
			} \
		} \
		\
		if(parseError) \
			v = vDefault; \
		\
		return *this; \
	}

ARGS_SHIFT_RIGHT(int32_t, 0)
ARGS_SHIFT_RIGHT(uint32_t, 0)
ARGS_SHIFT_RIGHT(int64_t, 0)
ARGS_SHIFT_RIGHT(uint64_t, 0)
ARGS_SHIFT_RIGHT(double, 0.0)
ARGS_SHIFT_RIGHT(std::string, std::string())
ARGS_SHIFT_RIGHT(const char*, "")

RPC_Message::RPC_Message() : packerBin(*this), packerTxt(*this), buf(NULL), size(0), filled(0), cur(0) {
	Resize(std::max(size_t(128), sizeof(MsgBinHeader)));
	cur = filled = sizeof(MsgBinHeader); // т.к. bin (см. Clear())
	
	packer = &packerBin;
}

RPC_Message::~RPC_Message() {
	if(buf)
		free(buf);
}

void RPC_Message::Clear() {
	if(packer == &packerBin) {
		cur = filled = sizeof(MsgBinHeader);
	}
	else {
		cur = filled = 0;
	}
}

void RPC_Message::Resize(size_t sizeNew) {
	if(sizeNew > size) {
		sizeNew += 64;
		UNP_Assert(sizeNew >= filled);
		buf = static_cast<uint8_t*>(UNP::realloc(buf, sizeNew));
		UNP_Assert(buf);
		size = sizeNew;
	}
}

void RPC_MessagePacker::GetVar_e(std::string &v) {
	const char *s;
	size_t len;
	
	GetVar_e(s, len);
	v.assign(s, len);
}

void RPC_MessagePacker::GetVar_e(const char* &v) {
	const char *s;
	size_t len;
	
	GetVar_e(s, len);
	v = s;
}

void RPC_MessagePacker_Bin::PutVar(const void *v, size_t len) {
	uint8_t *p = m.Put(len);
	
	memcpy(p, v, len);
}

void RPC_MessagePacker_Bin::PutVar(int32_t v) {
	v = htobe32(v);
	PutVar(&v, sizeof(v));
}

void RPC_MessagePacker_Bin::PutVar(uint32_t v) {
	v = htobe32(v);
	PutVar(&v, sizeof(v));
}

void RPC_MessagePacker_Bin::PutVar(int64_t v) {
	v = htobe64(v);
	PutVar(&v, sizeof(v));
}

void RPC_MessagePacker_Bin::PutVar(uint64_t v) {
	v = htobe32(v);
	PutVar(&v, sizeof(v));
}

void RPC_MessagePacker_Bin::PutVar(double v_) {
	static_assert(sizeof(double) == sizeof(int64_t), "");
	
	int64_t v = htobe64(*reinterpret_cast<int64_t*>(&v_));
	PutVar(&v, sizeof(v));
}

void RPC_MessagePacker_Bin::PutVar(const char *s, size_t n) {
	uint8_t *p = m.Put(n + 1);
	
	if(n > 0) {
		memcpy(p, s, n);
		p += n;
	}
	
	*p = '\0';
}

void RPC_MessagePacker_Bin::GetVar_e(void *v, size_t len) {
	uint8_t *p = m.Get_e(len);
	memcpy(v, p, len);
}

void RPC_MessagePacker_Bin::GetVar_e(int32_t &v) {
	GetVar_e(&v, sizeof(v));
	v = be32toh(v);
}

void RPC_MessagePacker_Bin::GetVar_e(uint32_t &v) {
	GetVar_e(&v, sizeof(v));
	v = be32toh(v);
}

void RPC_MessagePacker_Bin::GetVar_e(int64_t &v) {
	GetVar_e(&v, sizeof(v));
	v = be64toh(v);
}

void RPC_MessagePacker_Bin::GetVar_e(uint64_t &v) {
	GetVar_e(&v, sizeof(v));
	v = be64toh(v);
}

void RPC_MessagePacker_Bin::GetVar_e(double &v_) {
	static_assert(sizeof(double) == sizeof(int64_t), "");
	
	int64_t v;
	
	GetVar_e(&v, sizeof(v));
	v = be64toh(v);
	
	v_ = *reinterpret_cast<double*>(&v);
}

void RPC_MessagePacker_Bin::GetVar_e(const char* &v, size_t &len) {
	size_t filled;
	uint8_t *begin = m.CheckGet(&filled), *end;
	
	end = filled > 0 ? static_cast<uint8_t*>(memchr(begin, '\0', filled)) : NULL;
	if(!end) {
		l.AddError("Bin parse error: trailing '\\0' not found");
		throw 1;
	}
	
	v = reinterpret_cast<const char*>(begin);
	len = end - begin;
	
	m.IncrCur(end - begin + 1);
}

#define TXT_BUF_SIZE  32

#define TXT_PUT_VAR(type, format) \
	void RPC_MessagePacker_Txt::PutVar(type v) { \
		char *s = reinterpret_cast<char*>(m.ReservePut(TXT_BUF_SIZE)); \
		s[0] = 0; \
		snprintf(s, TXT_BUF_SIZE, format "\t", v); \
		m.IncrFilled(strlen(s)); \
	}

TXT_PUT_VAR(int32_t, "%" PRId32)
TXT_PUT_VAR(uint32_t, "0x%" PRIX32)
TXT_PUT_VAR(int64_t, "%" PRId64)
TXT_PUT_VAR(uint64_t, "0x%" PRIX64)
TXT_PUT_VAR(double, "%g")

void RPC_MessagePacker_Txt::PutVar(const char *s_, size_t n_) {
	std::string s("\"");
	
	for(size_t i = 0; i < n_; ++i) {
		if(s_[i] == '\"')
			s += '\\';
		s += s_[i];
	}
	
	s += "\" ";
	
	uint8_t *p = m.Put(s.length());
	memcpy(p, s.c_str(), s.length());
}

const char* RPC_MessagePacker_Txt::GetVar_e(size_t *plen, bool extended) {
	const char *prefix = "Txt parse error";
	size_t c;
	char *s = reinterpret_cast<char*>(m.CheckGet(&c));
	
	if(c <= 1) { // 1, т.к. в конце д.б. разделитель
		l.AddError("%s: %s", prefix, "data too small");
		throw 1;
	}
	
	char *begin = NULL;
	
	for(size_t i = 0; i < c; ++i) {
		if(!isspace(s[i])) {
			begin = s + i;
			break;
		}
	}
	
	if(!begin) {
		l.AddWarning("%s: %s", prefix, "initial none space not found");
		throw 1;
	}
	
	if(extended) {
		if(*begin == '"') {
			++begin;
		}
		else {
			extended = false;
		}
	}
	
	char *end = NULL;
	
	if(extended) {
		// В этом случае разделитель - кавычка, пробел не требуем
		
		bool inEsc = false;
		
		for(size_t i = begin - s /*+ 1*/; i < c; ++i) {
			if(inEsc) {
				inEsc = false;
			}
			else {
				if(s[i] == '\\') {
					if(i >= c - 1) {
						l.AddError("%s: %s", prefix, "incompleted escape sequence");
						throw 1;
					}
					
					inEsc = true;
					memmove(s + i, s + i + 1, c - i - 1);
					--i;
					--c;
				}
				else {
					if(s[i] == '"') {
						end = s + i;
						break;
					}
				}
			}
		}
	}
	else {
		for(size_t i = begin - s + 1; i < c; ++i) {
			if(isspace(s[i])) {
				end = s + i;
				break;
			}
		}
	}
	
	if(!end) {
		l.AddError("%s: %s", prefix, "separator not found");
		throw 1;
	}
	
	*end = 0;
	if(plen)
		*plen = end - begin;
	m.IncrCur(end - s + 1);
	return begin;
}

void RPC_MessagePacker_Txt::GetVar_e(int32_t &v) {
	const char *s = GetVar_e();
	char *end;
	
	static_assert(sizeof(long) >= sizeof(int32_t), "");
	
	v = strtol(s, &end, 0);
	if(end == s) {
		l.AddError("Parse INT32 error");
		throw 1;
	}
}

void RPC_MessagePacker_Txt::GetVar_e(uint32_t &v) {
	const char *s = GetVar_e();
	char *end;
	
	static_assert(sizeof(unsigned long) >= sizeof(uint32_t), "");
	
	v = strtoul(s, &end, 0);
	if(end == s) {
		l.AddError("Parse UINT32 error");
		throw 1;
	}
}

void RPC_MessagePacker_Txt::GetVar_e(int64_t &v) {
	const char *s = GetVar_e();
	char *end;
	
	static_assert(sizeof(long long) >= sizeof(int64_t), "");
	
	v = strtoll(s, &end, 0);
	if(end == s) {
		l.AddError("Parse INT64 error");
		throw 1;
	}
}

void RPC_MessagePacker_Txt::GetVar_e(uint64_t &v) {
	const char *s = GetVar_e();
	char *end;
	
	static_assert(sizeof(unsigned long long) >= sizeof(uint64_t), "");
	
	v = strtoull(s, &end, 0);
	if(end == s) {
		l.AddError("Parse UINT64 error");
		throw 1;
	}
}

void RPC_MessagePacker_Txt::GetVar_e(double &v) {
	const char *s = GetVar_e();
	char *end;
	
	v = strtod(s, &end);
	if(end == s) {
		l.AddError("Parse DOUBLE error");
		throw 1;
	}
}

void RPC_MessagePacker_Txt::GetVar_e(const char* &s, size_t &len) {
	s = GetVar_e(&len, true);
}

bool RPC_Message::Send(RPC_Socket &sock, unsigned cmd, bool method, bool toServer) {
	(void)toServer;
	
	if(packer == &packerTxt) {
		if(filled > 0) { // в конце гарантирован пробел, туда и запишем eol 
			buf[filled - 1] = '\n';
		}
		else {
			uint8_t *p = Put(1);
			*p = '\n';
		}
	}
	else {
		MsgBinHeader *h = reinterpret_cast<MsgBinHeader*>(buf);
		
		// h гарантированно не NULL, len >= sizeof(MsgHeader)
		
		h->prefix = MSG_BIN_HEADER_PREFIX;
		h->type = method ? MSG_BIN_HEADER_TYPE_METHOD : MSG_BIN_HEADER_TYPE_SIGNAL;
		h->cmd = htobe16(cmd);
		
		UNP_Assert(filled >= sizeof(MsgBinHeader));
		size_t dataLen = filled - sizeof(MsgBinHeader);
		UNP_Assert(dataLen <= UINT32_MAX);
		h->dataLen = htobe32(dataLen);
	}
	
	return Sock_WriteN(sock.sock, buf, filled);
}

// not NULL
static bool CheckUInt(const char *name, unsigned vWant, unsigned vRead) {
	if(vWant != vRead) {
		l.AddError("Parse '%s' error: want 0x%X read 0x%X", name, vWant, vRead);
		return false;
	}
	
	return true;
}

static inline bool TestModifyEol(uint8_t &ch) {
	switch(ch) {
		case '\n':
		case '\r':
		case '\0':
			ch = ' ';
			return true;
		default:
			return false;
	}
}

bool RPC_Message::Receive(RPC_Socket &sock, unsigned *pcmd, bool *pmethod, bool fromServer) {
	/*
	 * Общий try/catch делать нельзя, т.к. такая конструкция глючит при отмене внутри try:
	 * 
	 * static void* Test_th(void*) {
	 * while(1) {
	 * 	try {
	 * 		sleep(1);
	 * 	}
	 * 	catch(...) {
	 * 	}
	 * }
	 * 
	 * return NULL;
	 * }
	 */
	
	cur = filled = 0;
	
	Resize(sizeof(MsgBinHeader)); // можно без запаса, т.к. он уже есть в конструкторе
	
	MsgBinHeader *h = reinterpret_cast<MsgBinHeader*>(buf);
	// h гарантированно не NULL
	
	ssize_t r;
	
	// Чтение префикса или больше
	r = Sock_Read(sock.sock, buf, size, fromServer);
	if(r < 0) // ошибка
		return false;
	if(r == 0) // eof
		return false;
	
	filled = r;
	
	if(isascii(h->prefix) || h->prefix == 0xff) { // 0xff - telnet
		packer = &packerTxt;
		
		// Чтение до конца строки
		while(!TestModifyEol(buf[filled - 1])) {
			if(filled >= size)
				Resize(filled + 64);
			
			r = Sock_Read(sock.sock, buf + filled, size - filled, true);
			if(r < 0) // ошибка
				return false;
			if(r == 0) // eof
				return false;
			
			filled += r;
		}
		
		cur = 0;
		
		if(h->prefix == 0xff) {
			// В начале гарантированно !ascii (0xff), а в конце - ascii (eol заменён на пробел)
			for(ssize_t i = filled - 2; i >= 0; --i) {
				if(!isascii(buf[i])) {
					cur = i + 1;
					break;
				}
			}
		}
		
		*pmethod = true;
		
		uint32_t cmd;
		
		try {
			packerTxt.GetVar_e(cmd);
		}
		catch(...) {
			return false;
		}
		
		*pcmd = cmd;
	}
	else {
		size_t wantCount;
		
		packer = &packerBin;
		
		// Проверяем префикс
		if(!CheckUInt("prefix", MSG_BIN_HEADER_PREFIX, h->prefix))
			return false;
		
		// Дочитываем заголовок
		// (resize на размер заголовка не нужен, т.к. уже зарезервировано)
		wantCount = sizeof(MsgBinHeader);
		if(filled < wantCount) {
			if(!Sock_ReadN(sock.sock, buf + filled, wantCount - filled, true))
				return false;
			filled = wantCount;
		}
		
		h->cmd = be16toh(h->cmd);
		h->dataLen = be32toh(h->dataLen);
		
		// Проверяем остальные поля заголовка
		
		if(fromServer) {
			if(!CheckUInt("cmd", *pcmd, h->cmd))
				return false;
		}
		else
			*pcmd = h->cmd;
		
		if(fromServer) {
			if(!CheckUInt("type", *pmethod ? MSG_BIN_HEADER_TYPE_METHOD : MSG_BIN_HEADER_TYPE_SIGNAL, h->type))
				return false;
		}
		else {
			if(h->type == MSG_BIN_HEADER_TYPE_METHOD)
				*pmethod = true;
			else if(h->type == MSG_BIN_HEADER_TYPE_SIGNAL)
				*pmethod = false;
			else {
				l.AddError("Parse '%s' error: wrong value", "type");
				return false;
			}
		}
		
		// Дочитываем данные
		wantCount = sizeof(MsgBinHeader) + h->dataLen;
		if(filled < wantCount) {
			Resize(wantCount);
			if(!Sock_ReadN(sock.sock, buf + filled, wantCount - filled, true))
				return false;
			filled = wantCount;
		}
		
		if(filled > wantCount) {
			l.AddError("Parse error: message too long");
			return false;
		}
		
		cur = sizeof(*h);
	}
	
	return true;
}

}
