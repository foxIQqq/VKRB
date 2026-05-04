/*
 * unp_rpc_server_impl.h
 *
 *  Created on: 18 апр. 2022 г.
 *      Author: kudr
 */

#ifndef UNP_RPC_SERVER_IMPL_H_
#define UNP_RPC_SERVER_IMPL_H_

#include <string.h>
#include <list>

#include "unp_rpc_server.h"
#include "unp_rpc_private.h"

namespace UNP {

class RPC_Server_Settings {
public:
	std::string bindAddress;
	unsigned short bindPort;
	
	RPC_Server_Callback_ClientRequest_f cbClientRequest;
	void* cbClientRequestData;
	
	RPC_Server_Settings(const char *bindAddress_, unsigned short bindPort_, RPC_Server_Callback_ClientRequest_f cbClientRequest_, void* cbClientRequestData_) : bindAddress(bindAddress_), bindPort(bindPort_), cbClientRequest(cbClientRequest_), cbClientRequestData(cbClientRequestData_) {}
	RPC_Server_Settings() : bindPort(0), cbClientRequest(NULL), cbClientRequestData(NULL) {}
};

class RPC_ServerImpl {
public:
	RPC_Server_Settings sets;
	bool isLocal;
	
	pthread_t acceptTid;
	bool acceptTidOk;
	
	RPC_Socket sockListen; // работаем и закрываем в потоке
	
public:
	// pbindPort == NULL => локальный сокет, а address - путь
	RPC_ServerImpl(const char *bindAddress, unsigned short *pbindPort, RPC_Server_Callback_ClientRequest_f cbClientRequest, void *cbClientRequestData);
	~RPC_ServerImpl();
	
	bool Start(bool createThread);
	void Stop();
	
	inline bool IsLocal() const { return isLocal; }
};

namespace RPC_SERVER {

enum CmdHandleThreadState {
	CHTS_AVL = 0, // Поток не запущен, слот свободен
	CHTS_BUSY, // Поток работает, слот занят
	CHTS_ZOMBIE // Слот освободился, но синхронизации (pthread_join()) не было
};

class SharedData;

// Информация о i-м обработчике команд клиента
class CmdHandleInfo {
public:
	SharedData *pshared; // not NULL
	
	// с блокировкой
	CmdHandleThreadState state;
	pthread_t tid;
	
	// без блокировки
	RPC_Socket sock; // иниц-ся потоком RPC_Server_CmdAccept_th(), потом с этой переменной работает только поток RPC_Server_CmdHandle_th
	
	CmdHandleInfo(SharedData &shared) : pshared(&shared), state(CHTS_AVL) { bzero(&tid, sizeof(tid)); }
};

// Общие данные
// Для всего, кроме psets, нужны блокировки
class SharedData {
public:
	// с блокировками
	std::list<CmdHandleInfo*> cmdHandlers;
	int cmdHandlersBusyCount; // количество занятых обработчиков (м.б. < cmdHandlers.get_count())
	
	// без блокировок
	const RPC_Server_Settings *psets;
	
public:
	SharedData(const RPC_Server_Settings &sets);
	~SharedData();
	
	void Lock() { condMutex.Lock(); }
	void Unlock() { condMutex.Unlock(); }
	
	void Wait() { condMutex.Wait(); }
	void Trigger() { condMutex.Trigger(); }
	
	void WaitCmdHandlerAvlAll(); // Ждём освобождения всех обработчиков
	CmdHandleInfo* GetCmdHandlerAvl_lock(); // (мьютекс останется заблокированным)
	
private:
	CondMutex condMutex;
};

} // namespace RPC_SERVER

using namespace RPC_SERVER;

// Потоки. Могут быть принудительно завершены вызовом pthread_cancel().

// Приём команд от клиентов
void* RPC_Server_CmdAccept_th(void*);
// Обработка команд от клиентов
void* RPC_Server_CmdHandle_th(void*);

}
#endif /* UNP_RPC_SERVER_IMPL_H_ */
