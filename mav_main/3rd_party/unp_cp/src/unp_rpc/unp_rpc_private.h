/*
 * unp_rpc_impl.h
 *
 *  Created on: 05.02.2014
 *      Author: kudr
 */

#ifndef UNP_RPC_PRIVATE_H_
#define UNP_RPC_PRIVATE_H_

#include "unp_threads.h"
#include "unp_sock.h"
#include "unp_private.h"

#include "unp_rpc.h"

namespace UNP {

class RPC_Socket {
public:
	int sock = -1;
	
public:
	~RPC_Socket() { Close(); }
	
	void Close(const char *localPath = NULL) {
		if(IsValid()) {
			Sock_Close(sock, localPath);
			sock = -1;
		} 
	}
	inline bool IsValid() const { return sock >= 0; }
};

class RPC_Message;

class RPC_MessagePacker {
protected:
	RPC_Message &m;
	
public:
	RPC_MessagePacker(RPC_Message &m_) : m(m_) {}
	virtual ~RPC_MessagePacker() {}
	
	virtual void PutVar(int32_t)                 = 0;
	virtual void PutVar(uint32_t)                = 0;
	virtual void PutVar(int64_t)                 = 0;
	virtual void PutVar(uint64_t)                = 0;
	virtual void PutVar(double)                  = 0;
	virtual void PutVar(const char *s, size_t n) = 0; // аргументы д.б. корректными
	
	virtual void GetVar_e(int32_t &)                   = 0;
	virtual void GetVar_e(uint32_t &)                  = 0;
	virtual void GetVar_e(int64_t &)                   = 0;
	virtual void GetVar_e(uint64_t &)                  = 0;
	virtual void GetVar_e(double &)                    = 0;
	virtual void GetVar_e(const char* &s, size_t &len) = 0; // s будет заканч-ся нулём, len - как бонус, т.к. всё равно вычислено
	
	void GetVar_e(std::string &) ;
	void GetVar_e(const char* &) ;
};

class RPC_MessagePacker_Bin : public RPC_MessagePacker {
public:
	RPC_MessagePacker_Bin(RPC_Message &m_) : RPC_MessagePacker(m_) {}
	
	virtual void PutVar(int32_t);
	virtual void PutVar(uint32_t);
	virtual void PutVar(int64_t);
	virtual void PutVar(uint64_t);
	virtual void PutVar(double);
	virtual void PutVar(const char *s, size_t n);
	
	virtual void GetVar_e(int32_t &);
	virtual void GetVar_e(uint32_t &);
	virtual void GetVar_e(int64_t &);
	virtual void GetVar_e(uint64_t &);
	virtual void GetVar_e(double &);
	virtual void GetVar_e(const char* &s, size_t &len);
	
private:
	void PutVar(const void *v, size_t len);
	
	void GetVar_e(void *v, size_t len);
};

class RPC_MessagePacker_Txt : public RPC_MessagePacker {
public:
	RPC_MessagePacker_Txt(RPC_Message &m_) : RPC_MessagePacker(m_) {}
	
	virtual void PutVar(int32_t);
	virtual void PutVar(uint32_t);
	virtual void PutVar(int64_t);
	virtual void PutVar(uint64_t);
	virtual void PutVar(double);
	virtual void PutVar(const char *s, size_t n);
	
	virtual void GetVar_e(int32_t &);
	virtual void GetVar_e(uint32_t &);
	virtual void GetVar_e(int64_t &);
	virtual void GetVar_e(uint64_t &);
	virtual void GetVar_e(double &);
	virtual void GetVar_e(const char* &s, size_t &len);
	
private:
	const char* GetVar_e(size_t *plen = NULL, bool extended = false);
};

class RPC_Message {
public:
	RPC_MessagePacker *packer;
	
public:
	RPC_Message();
	~RPC_Message();
	
	void Clear();
	
	void SetPackerType(bool isBin) {
		if(isBin)
			packer = &packerBin;
		else
			packer = &packerTxt;
	}
	bool GetPackerType() const { return packer == &packerBin; }
	
	bool Send(RPC_Socket &sock, unsigned cmd, bool method, bool toServer);
	bool Receive(RPC_Socket &sock, unsigned *pcmd, bool *pmethod, bool fromServer); // p* not NULL
	
	uint8_t* Put(size_t bytesCount) {
		size_t prev = filled, next = filled + bytesCount;
		
		Resize(next);
		filled = next;
		return buf + prev;
	}
	
	uint8_t* Get_e(size_t bytesCount) {
		if(cur + bytesCount > filled) {
			l.AddError("%s: %s", __func__, "data too small");
			throw 1;
		}
		
		size_t prev = cur;
		
		cur += bytesCount;
		
		return buf + prev;
	}
	
	uint8_t* CheckGet(size_t *pavlBytesCount) {
		*pavlBytesCount = filled >= cur ? filled - cur : 0;
		return buf + cur;
	}
	
	void IncrCur(size_t incr) { cur += incr; }
	
	uint8_t* ReservePut(size_t bytesCount) {
		Resize(filled + bytesCount);
		return buf + filled;
	}
	
	void IncrFilled(size_t incr) { filled += incr; }
	
private:
	RPC_MessagePacker_Bin packerBin;
	RPC_MessagePacker_Txt packerTxt;
	
	uint8_t *buf;
	size_t size, filled, cur;
	
private:
	void Resize(size_t sizeNew); // в случае ошибки - UNP_Assert()
};

}
#endif
