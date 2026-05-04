/*
 * rpc_client_impl.h
 *
 *  Created on: 18 апр. 2022 г.
 *      Author: kudr
 */

#ifndef UNP_RPC_CLIENT_IMPL_H_
#define UNP_RPC_CLIENT_IMPL_H_

#include "unp_rpc_private.h"

namespace UNP {

class RPC_ClientImpl {
public:
	// pport == NULL => локальный сокет, а address - путь
	RPC_ClientImpl(const char *address, unsigned short *pport, int connTimeout_msec, int talkTimeout_msec);
	
	bool Talk(unsigned cmd, bool method, RPC_Args *pinArgs, RPC_Args *poutArgs);
	inline bool IsLocal() const { return isLocal; }
	void SetAttempts(int attempts) { attemptsMax = attempts; }
	
private:
	std::string address;
	unsigned short port;
	int connTimeout_msec, talkTimeout_msec;
	bool isLocal;
	int attemptsMax;
	
	RPC_Socket sock;
	
	// используем, если в Talk() соотв-й *msg == NULL (объявил в классе, а не в Talk() для уменьшения alloc-ов)
	RPC_Message sendMsg0;
	RPC_Message recvMsg0;
	
private:
	bool Talk_single(unsigned cmd, bool method, RPC_Message &sendMsg, RPC_Message &recvMsg, bool *pcanRepeat); // p* not NULL
};

}
#endif /* UNP_RPC_CLIENT_IMPL_H_ */
