/*
 * unp_rpc_client.h
 *
 * Клиент UNP RPC (подробности в unp_rpc.h).
 *
 *  Created on: 18 апр. 2022 г.
 *      Author: kudr
 */

#ifndef INC_UNP_RPC_CLIENT_H_
#define INC_UNP_RPC_CLIENT_H_

#include "unp_rpc.h"

namespace UNP {

class RPC_ClientImpl;

/*!
 * Базовый класс клиента.
 * 
 * Сообщения бывают двух типов: методы (method) и сингалы (signal).
 * 
 * Метод: сервер сначала вызовет обработчик, который сформирует ответ, а потом ответит клиенту.
 * Сигнал: сервер сначала ответит (пустым сообщением, естественно), а потом вызовет обработчик.
 */
class RPC_Client {
public:
	static constexpr int ATTEMPTS_DEFAULT = 2;
	
	/*!
	 * Конструктор.
	 * Включается игнорирование SIGPIPE.
	 */
	RPC_Client(RPC_ClientImpl *impl);
	
	virtual ~RPC_Client();
	
	RPC_Client(const RPC_Client &) = delete;
	RPC_Client &operator=(const RPC_Client &) = delete;
	
	/*!
	 * Вызов метода
	 * 
	 * @param cmd       Код команды
	 * @param pinArgs   Входные аргументы (NULL - отстутствуют)
	 * @param poutArgs  Выходные аргументы (NULL - ответ не интересует)
	 * 
	 * @return Успех / ошибка
	 */
	virtual bool MethodCall(unsigned cmd, RPC_Args *pinArgs, RPC_Args *poutArgs) = 0;
	
	/*!
	 * Отправка сигнала
	 * 
	 * @param cmd      Код команды
	 * @param pinArgs  Входные аргументы (NULL - отстутствуют)
	 * 
	 * @return Успех / ошибка
	 */
	virtual bool SignalSend(unsigned cmd, RPC_Args *pinArgs) = 0;
	
	/*!
	 * Задать количество попыток
	 * @param attempts  Количество попыток
	 */
	void SetAttempts(int attempts);
	
protected:
	RPC_ClientImpl *impl;
};

/*!
 * Базовый класс клиента для работы через сокеты
 */
class RPC_Client_Socket : public RPC_Client {
public:
	RPC_Client_Socket(RPC_ClientImpl *impl) : RPC_Client(impl) {}
	virtual ~RPC_Client_Socket();
	
	bool MethodCall(unsigned cmd, RPC_Args *pinArgs, RPC_Args *poutArgs) override;
	bool SignalSend(unsigned cmd, RPC_Args *pinArgs) override;
};

/*!
 * Клиент, работающий по TCP
 */
class RPC_Client_TCP : public RPC_Client_Socket {
public:
	/*!
	 * Конструктор.
	 * 
	 * Фактическое подключение происходит не в конструкторе,
	 * а при первом вызове MethodCall(), SignalSend().
	 * 
	 * @param address           Адрес (IP-адрес или имя узла)
	 * @param port              Порт
	 * @param connTimeout_msec  Таймаут подключения (мс; <0 - не устанавливать, 0 - именно 0)
	 * @param talkTimeout_msec  Таймаут обмена (мс; <0 - не устанавливать, 0 - без ограничения по времени)
	 */
	RPC_Client_TCP(const char *address, unsigned short port, int connTimeout_msec, int talkTimeout_msec);
};

/*!
 * Клиент, работающий через локальные потоковые сокеты
 */
class RPC_Client_LocalStream : public RPC_Client_Socket {
public:
	/*!
	 * Конструктор.
	 * 
	 * Как и для TCP, подключение происходит не в конструкторе,
	 * а при первом вызове MethodCall(), SignalSend().
	 * 
	 * @param path              Путь
	 * @param connTimeout_msec  Таймаут подключения (мс; <0 - не устанавливать, 0 - именно 0)
	 * @param talkTimeout_msec  (мс; <0 - не устанавливать, 0 - без ограничения по времени)
	 */
	RPC_Client_LocalStream(const char *path, int connTimeout_msec, int talkTimeout_msec);
};

}
#endif /* INC_UNP_RPC_CLIENT_H_ */
