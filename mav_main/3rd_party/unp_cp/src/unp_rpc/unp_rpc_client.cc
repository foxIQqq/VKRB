/*
 * unp_rpc_client.cc
 *
 *  Created on: 03.02.2014
 *      Author: kudr
 */

#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "unp_misc/signal.h"
#include "unp_time.h"
#include "unp_private.h"

#include "unp_rpc_client.h"
#include "unp_rpc_client_impl.h"

#ifndef SOL_TCP
#define SOL_TCP  IPPROTO_TCP
#endif

namespace UNP {

RPC_Client::RPC_Client(RPC_ClientImpl *impl_) : impl(impl_) {
	static struct Ignore {
		Ignore() {
			UNP::signal_r(SIGPIPE, SIG_IGN);
		}
	} ign;
}

RPC_Client::~RPC_Client() {
	delete impl;
}

void RPC_Client::SetAttempts(int attempts) {
	impl->SetAttempts(attempts);
}

RPC_Client_Socket::~RPC_Client_Socket() {
}

bool RPC_Client_Socket::MethodCall(unsigned cmd, RPC_Args *pinArgs, RPC_Args *poutArgs) {
	return impl->Talk(cmd, true, pinArgs, poutArgs);
}

bool RPC_Client_Socket::SignalSend(unsigned cmd, RPC_Args *pinArgs) {
	return impl->Talk(cmd, false, pinArgs, NULL);
}

RPC_Client_TCP::RPC_Client_TCP(const char *address, unsigned short port, int connTimeout_msec, int talkTimeout_msec)
	: RPC_Client_Socket(new RPC_ClientImpl(address, &port, connTimeout_msec, talkTimeout_msec))
{
}

RPC_Client_LocalStream::RPC_Client_LocalStream(const char *path, int connTimeout_msec, int talkTimeout_msec)
	: RPC_Client_Socket(new RPC_ClientImpl(path, NULL, connTimeout_msec, talkTimeout_msec))
{
}

RPC_ClientImpl::RPC_ClientImpl(const char *address_, unsigned short *pport_, int connTimeout_msec_, int talkTimeout_msec_) : address(address_ ? address_ : ""), port(pport_ ? *pport_ : 0), connTimeout_msec(connTimeout_msec_), talkTimeout_msec(talkTimeout_msec_), isLocal(pport_ == NULL), attemptsMax(RPC_Client::ATTEMPTS_DEFAULT) {
}

bool RPC_ClientImpl::Talk(unsigned cmd, bool method, RPC_Args *pinArgs, RPC_Args *poutArgs) {
	const char *func_name = "RPC_ClientImpl::Talk";
	
	int attempts = attemptsMax;
	bool ok = true, canRepeat;
	RPC_Message *psendMsgImpl = pinArgs ? pinArgs->message : &sendMsg0;
	RPC_Message *precvMsgImpl = poutArgs ? poutArgs->message : &recvMsg0;
	
	do {
		if(!ok)
			l.AddWarning("%s: Retry", func_name);
		ok = Talk_single(cmd, method, *psendMsgImpl, *precvMsgImpl, &canRepeat);
		if(!ok) {
			sock.Close();
			if(!canRepeat)
				attempts = 0;
		}
	} while(!ok && --attempts > 0);
	
	return ok;
}

static int ConnectAttemptsTimeout(bool isLocal, const char *address, const char *servName, int timeout_ms) {
	int64_t tBegin = UNP::GetTime(UNP::TM_MS);
	
	for(;;) {
		int fd = isLocal ? SockLocalStream_Connect(address, timeout_ms) : SockTCP_Connect_names(address, servName, timeout_ms);
		if(fd >= 0)
			return fd;
		
		int64_t t = UNP::GetTime(UNP::TM_MS);
		if(t - tBegin >= timeout_ms)
			break;
		
		constexpr int tSleep_ms = 250;
		
		l.AddNote("Can't connect; retry after %d ms", tSleep_ms);
		usleep(tSleep_ms * 1000);
	}
	
	l.AddError("Can't connect during %d ms", timeout_ms);
	return -1;
}

bool RPC_ClientImpl::Talk_single(unsigned cmd, bool method, RPC_Message &sendMsg, RPC_Message &recvMsg, bool *pcanRepeat) {
	*pcanRepeat = false;
	
	const int ONE = 1;
	bool isLocal = IsLocal();
	
	// open, connect
	if(!sock.IsValid()) {
		if(isLocal) {
			sock.sock = connTimeout_msec > 0 ? ConnectAttemptsTimeout(isLocal, address.c_str(), NULL, connTimeout_msec) : SockLocalStream_Connect(address.c_str(), connTimeout_msec);
			if(!sock.IsValid())
				return false;
		}
		else {
			char buf[16];
			
			buf[0] = 0;
			snprintf(buf, sizeof(buf), "%u", port);
			
			sock.sock = connTimeout_msec > 0 ? ConnectAttemptsTimeout(isLocal, address.c_str(), buf, connTimeout_msec) : SockTCP_Connect_names(address.c_str(), buf, connTimeout_msec);
			if(!sock.IsValid())
				return false;
			
			if(!Sock_SetOpt(sock.sock, SOL_TCP, "TCP_NODELAY", TCP_NODELAY, &ONE, sizeof(ONE)))
				return false;
		}
	}
	
	if(!Sock_SetTimeouts(sock.sock, talkTimeout_msec))
		return false;
	
	*pcanRepeat = true;
	
	// write
	if(!sendMsg.Send(sock, cmd, method, true))
		return false;
	
	// read
	if(!recvMsg.Receive(sock, &cmd, &method, true))
		return false;
	
	return true;
}

}
