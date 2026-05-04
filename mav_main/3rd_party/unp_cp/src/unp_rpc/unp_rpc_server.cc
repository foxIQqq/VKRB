/*
 * unp_rpc_server.cc
 *
 *  Created on: 03.02.2014
 *      Author: kudr
 */

#include "unp_misc/signal.h"
#include "unp_assert.h"
#include "unp_private.h"

#include "unp_rpc_server.h"
#include "unp_rpc_server_impl.h"

namespace UNP {

#define SERVER  (static_cast<RPC_ServerImpl*>(impl))

RPC_Server::RPC_Server() {
	static struct Ignore {
		Ignore() {
			UNP::signal_r(SIGPIPE, SIG_IGN);
		}
	} ign;
}

RPC_Server_Socket::~RPC_Server_Socket() {
	delete SERVER;
}

bool RPC_Server_Socket::Start(bool createThread) {
	return SERVER->Start(createThread);
}

void RPC_Server_Socket::Stop() {
	return SERVER->Stop();
}

RPC_Server_TCP::RPC_Server_TCP(const char *bindAddress, unsigned short bindPort, RPC_Server_Callback_ClientRequest_f cbClientRequest, void *cbClientRequestData)
	: RPC_Server_Socket(new RPC_ServerImpl(bindAddress, &bindPort, cbClientRequest, cbClientRequestData))
{
}

RPC_Server_LocalStream::RPC_Server_LocalStream(const char *bindPath, RPC_Server_Callback_ClientRequest_f cbClientRequest, void *cbClientRequestData)
	: RPC_Server_Socket(new RPC_ServerImpl(bindPath, NULL, cbClientRequest, cbClientRequestData))
{
}

RPC_ServerImpl::RPC_ServerImpl(const char *bindAddress_, unsigned short *pbindPort_, RPC_Server_Callback_ClientRequest_f cbClientRequest_, void* cbClientRequestData_) : sets(bindAddress_ ? bindAddress_ : "", pbindPort_ ? *pbindPort_ : 0, cbClientRequest_, cbClientRequestData_), isLocal(pbindPort_ == NULL), acceptTid(0), acceptTidOk(false) {
	UNP_Assert(sets.cbClientRequest);
}

RPC_ServerImpl::~RPC_ServerImpl() {
	Stop();
}

void RPC_ServerImpl::Stop() {
	if(acceptTidOk) {
		pthread_cancel(acceptTid);
		pthread_join(acceptTid, NULL);
		acceptTidOk = false;
	}
}

bool RPC_ServerImpl::Start(bool createThread) {
	if(acceptTidOk) // уже запущен
		return true;
	
	UNP_Assert(!sockListen.IsValid());
	
	// Пробуем забиндиться до запуска потока
	if(IsLocal())
		sockListen.sock = SockLocalStream_Listen(sets.bindAddress.c_str(), 10);
	else
		sockListen.sock = SockTCP_Listen(sets.bindAddress.c_str(), sets.bindPort, 10);
	
	if(sockListen.sock >= 0) {
		if(createThread) {
			if(ThreadCreate(&acceptTid, RPC_Server_CmdAccept_th, this)) {
				acceptTidOk = true;
				return true;
			}
			else {
				sockListen.Close(IsLocal() ? sets.bindAddress.c_str() : NULL);
			}
		}
		else {
			RPC_Server_CmdAccept_th(this);
			return true;
		}
	}
	
	return false;
}

SharedData::SharedData(const RPC_Server_Settings &sets) : cmdHandlersBusyCount(0), psets(&sets) {
}

SharedData::~SharedData() {
	std::list<CmdHandleInfo*>::iterator it;
	for(it = cmdHandlers.begin(); it != cmdHandlers.end(); ++it) {
		delete *it;
	}
}

void SharedData::WaitCmdHandlerAvlAll() {
	ThreadCancelDisabler dis;
	
	Lock();
	while(cmdHandlersBusyCount > 0)
		Wait();
	
	// В этой точке слоты либо в состоянии CHTS_AVL, либо CHTS_ZOMBIE
	
	std::list<CmdHandleInfo*>::iterator it;
	for(it = cmdHandlers.begin(); it != cmdHandlers.end(); ++it) {
		if((*it)->state == CHTS_ZOMBIE) {
			pthread_join((*it)->tid, NULL);
			(*it)->state = CHTS_AVL;
		}
	}
	
	Unlock();
}

CmdHandleInfo* SharedData::GetCmdHandlerAvl_lock() {
	CmdHandleInfo *handle = NULL;
	
	Lock();
	
	// Найдём свободный слот
	
	std::list<CmdHandleInfo*>::iterator it;
	for(it = cmdHandlers.begin(); it != cmdHandlers.end(); ++it) {
		if((*it)->state != CHTS_BUSY) {
			if((*it)->state == CHTS_ZOMBIE) {
				pthread_join((*it)->tid, NULL);
				(*it)->state = CHTS_AVL;
			}
			
			handle = (*it);
			break;
		}
	}
	
	if(!handle) {
		handle = new CmdHandleInfo(*this);
		cmdHandlers.push_back(handle);
	}
	
	return handle;
}

}
