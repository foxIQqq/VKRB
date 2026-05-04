/*
 * unp_rpc_server_cmd_accept.cc
 *
 *  Created on: 03.02.2014
 *      Author: kudr
 */

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "unp_assert.h"
#include "unp_private.h"

#include "unp_rpc_server_impl.h"

#ifndef SOL_TCP
#define SOL_TCP  IPPROTO_TCP
#endif

namespace UNP {

// Всё, что нужно для корректного завершения по pthread_cancel
class CmdAcceptData {
public:
	// без блокировок
	RPC_ServerImpl *pserver; // not NULL
	RPC_Socket sockConn;
	
	// с блокировками
	SharedData shared;
	
	CmdAcceptData(RPC_ServerImpl &server);
	~CmdAcceptData();
	
	static void Delete(void *thiz) {
		delete static_cast<CmdAcceptData*>(thiz);
	}
};

CmdAcceptData::CmdAcceptData(RPC_ServerImpl &server) : pserver(&server), shared(server.sets) {
	if(pserver->IsLocal())
		l.AddDebug("RPC CmdAccept start ('%s')", pserver->sets.bindAddress.c_str());
	else
		l.AddDebug("RPC CmdAccept start (%s:%u)", pserver->sets.bindAddress.c_str(), (unsigned)pserver->sets.bindPort);
}

CmdAcceptData::~CmdAcceptData() {
	ThreadCancelDisabler dis;
	std::list<CmdHandleInfo*>::iterator it;
	
	// Остановим потоки, обрабатывающие запросы
	shared.Lock();
	for(it = shared.cmdHandlers.begin(); it != shared.cmdHandlers.end(); ++it) {
		if((*it)->state == CHTS_BUSY)
			pthread_cancel((*it)->tid);
	}
	shared.Unlock();
	
	// Ждём завершения
	shared.WaitCmdHandlerAvlAll();
	
	sockConn.Close();
	pserver->sockListen.Close(pserver->IsLocal() ? pserver->sets.bindAddress.c_str() : NULL);
	
	l.AddDebug("RPC CmdAccept stop");
}

void* RPC_Server_CmdAccept_th(void *server_) {
	RPC_ServerImpl *server = static_cast<RPC_ServerImpl*>(server_);
	CmdAcceptData *data = new CmdAcceptData(*server);
	
	pthread_setname_np(pthread_self(), "unp:rpc_server:cmd_accept");
	pthread_cleanup_push(CmdAcceptData::Delete, data);
	
	CmdHandleInfo *cmdHandle;
	int one = 1;
	
	for(;;) {
		UNP_Assert(!data->sockConn.IsValid());
		
		// Ждём подключения
		data->sockConn.sock = data->pserver->IsLocal() ? SockLocalStream_Accept(data->pserver->sockListen.sock) : SockTCP_Accept(data->pserver->sockListen.sock);
		if(data->sockConn.IsValid()) {
			// Клиент подключился
			if(data->pserver->IsLocal() || Sock_SetOpt(data->sockConn.sock, SOL_TCP, "TCP_NODELAY", TCP_NODELAY, &one, sizeof(one))) {
				// Получим указатель на свободный слот
				cmdHandle = data->shared.GetCmdHandlerAvl_lock();
				UNP_Assert(cmdHandle);
				
				cmdHandle->sock = data->sockConn;
				if(ThreadCreate(&cmdHandle->tid, RPC_Server_CmdHandle_th, cmdHandle)) {
					cmdHandle->state = CHTS_BUSY;
					data->shared.cmdHandlersBusyCount++;
					data->sockConn.sock = -1; // т.к. закроется в созданном потоке
				}
				data->shared.Unlock();
			}
			
			data->sockConn.Close();
		}
	}
	
	pthread_cleanup_pop(1);
	return NULL;
}

}
