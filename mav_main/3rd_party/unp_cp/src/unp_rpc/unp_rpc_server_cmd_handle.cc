/*
 * unp_rpc_server_cmd_handle.cc
 *
 *  Created on: 03.02.2014
 *      Author: kudr
 */

#include "unp_private.h"

#include "unp_rpc_server.h"
#include "unp_rpc_server_impl.h"

namespace UNP {

// Всё, что нужно для корректного завершения по pthread_cancel()
class CmdHandleData {
public:
	CmdHandleInfo *info;
	RPC_Args req, answ;
	
	CmdHandleData(CmdHandleInfo &info_);
	~CmdHandleData();
	
	static void Delete(void *thiz) {
		delete static_cast<CmdHandleData*>(thiz);
	}
};

CmdHandleData::CmdHandleData(CmdHandleInfo &info_) : info(&info_) {
	l.AddDebug("RPC CmdHandle (tid=%u): start", (unsigned)pthread_self());
}

CmdHandleData::~CmdHandleData() {
	ThreadCancelDisabler dis;
	
	info->sock.Close();
	
	info->pshared->Lock();
	info->state = CHTS_ZOMBIE;
	info->pshared->cmdHandlersBusyCount--;
	info->pshared->Unlock();
	info->pshared->Trigger();
	
	l.AddDebug("RPC CmdHandle (tid=%u): stop", (unsigned)pthread_self());
}

void* RPC_Server_CmdHandle_th(void *userData) {
	CmdHandleData *data = new CmdHandleData(*static_cast<CmdHandleInfo*>(userData));
	
	pthread_setname_np(pthread_self(), "unp:rpc_server:cmd_handle");
	pthread_cleanup_push(CmdHandleData::Delete, data);
	
	bool ok = true, method;
	unsigned cmd;
	RPC_Args &req = data->req, &answ = data->answ;
	CmdHandleInfo *info = data->info;
	
	while(ok) {
		req.Clear();
		ok = req.message->Receive(info->sock, &cmd, &method, false);
		if(ok) {
			answ.message->SetPackerType(req.message->GetPackerType());
			answ.Clear();
			
			if(method) {
				UNP::SetCancelState(PTHREAD_CANCEL_DISABLE, NULL);
				info->pshared->psets->cbClientRequest(cmd, req, answ, info->pshared->psets->cbClientRequestData);
				UNP::SetCancelState(PTHREAD_CANCEL_ENABLE, NULL);
				
				ok = answ.message->Send(info->sock, cmd, method, false);
			}
			else {
				ok = answ.message->Send(info->sock, cmd, method, false);
				
				UNP::SetCancelState(PTHREAD_CANCEL_DISABLE, NULL);
				info->pshared->psets->cbClientRequest(cmd, req, answ, info->pshared->psets->cbClientRequestData);
				UNP::SetCancelState(PTHREAD_CANCEL_ENABLE, NULL);
			}
		}
	}
	
	pthread_cleanup_pop(1);
	return NULL;
}

}
