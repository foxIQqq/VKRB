/*
 * unp_rpc_server.h
 *
 * Сервер UNP RPC (подробности в unp_rpc.h).
 *
 *  Created on: 18 апр. 2022 г.
 *      Author: kudr
 */

#ifndef INC_UNP_RPC_SERVER_H_
#define INC_UNP_RPC_SERVER_H_

#include "unp_rpc.h"

namespace UNP {

/*!
 * Callback для обработки запроса клиента
 * 
 * @param cmd       Команда
 * @param inArgs    Запрос
 * @param outArgs   Сюда положить ответ (перед заполнением Clear() можно не вызывать) (для сигнала не используется)
 * @param userData  Произвольные пользовательские данные
 */
typedef void (*RPC_Server_Callback_ClientRequest_f) (unsigned cmd, RPC_Args &inArgs, RPC_Args &outArgs, void *userData);

//! Базовый класс сервера
class RPC_Server {
public:
	/*!
	 * Конструктор.
	 * Включается игнорирование SIGPIPE.
	 */
	RPC_Server();
	
	virtual ~RPC_Server() {}
	
	RPC_Server(const RPC_Server &) = delete;
	RPC_Server &operator=(const RPC_Server &) = delete;
	
	/*!
	 * Запуск сервера.
	 * 
	 * Если background - true, то сервер будет работать в фоновом режиме в отдельном потоке.
	 * Метод вернёт true в случае успешного запуска, false - при ошибке.
	 * Останов - Stop() или деструктор.
	 * 
	 * Если background - false, то сервер будет работать в вызвавшем потоке.
	 * В случае успеха возврата из этого метода не будет, при ошибке вернёт false.
	 * Останавливается вызовом pthread_cancel() для вызвавшего потока
	 * (вызывать Stop() или деструктор из другого потока нельзя, т.к. в библиотеке не поддерживается работа с одним экземпляром из разных потоков).
	 * 
	 * При поступлении запросов от клиентов будет асинхронно вызываться cbClientRequest().
	 */
	virtual bool Start(bool background) = 0;
	
	/// Останов сервера
	virtual void Stop() = 0;
};

/*!
 * Базовый класс сервера, работающего через сокеты
 */
class RPC_Server_Socket : public RPC_Server {
public:
	RPC_Server_Socket(void *impl_) : RPC_Server(), impl(impl_) {}
	virtual ~RPC_Server_Socket();
	
	bool Start(bool background) override;
	void Stop() override;
	
private:
	void *impl;
};

/*!
 * Сервер, работающий по TCP
 */
class RPC_Server_TCP : public RPC_Server_Socket {
public:
	/*!
	 * Конструктор.
	 * 
	 * Фактическое создание сервера происходит не в конструкторе,
	 * а в Start().
	 * 
	 * @param bindAddress          Адрес
	 * @param bindPort             Порт
	 * @param cbClientRequest      Callback для обработки запросов (не NULL).
	 *                             Перед вызовом отключается отмена потока (PTHREAD_CANCEL_DISABLE). Внутри включать (PTHREAD_CANCEL_ENABLE) можно.
	 * @param cbClientRequestData  Пользовательские данные для callback-а
	 */
	RPC_Server_TCP(const char *bindAddress, unsigned short bindPort, RPC_Server_Callback_ClientRequest_f cbClientRequest, void *cbClientRequestData);
};

/*!
 * Сервер, работающий через локальные потоковые сокеты
 */
class RPC_Server_LocalStream : public RPC_Server_Socket {
public:
	/*!
	 * Конструктор.
	 * 
	 * Как и для TCP, фактически сервер создаётся не в конструкторе,
	 * а в Start().
	 * 
	 * @param bindPath             Путь
	 * @param cbClientRequest      Callback для обработки запросов (не NULL).
	 *                             Перед вызовом отключается отмена потока (PTHREAD_CANCEL_DISABLE). Внутри включать (PTHREAD_CANCEL_ENABLE) можно.
	 * @param cbClientRequestData  Пользовательские данные для callback-а
	 */
	RPC_Server_LocalStream(const char *bindPath, RPC_Server_Callback_ClientRequest_f cbClientRequest, void *cbClientRequestData);
};

}
#endif /* INC_UNP_RPC_SERVER_H_ */
