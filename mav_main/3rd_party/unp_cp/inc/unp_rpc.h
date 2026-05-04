/*
 * unp_rpc.h
 * 
 * Простой механизм обмена сообщениями.
 * Многопоточность поддерживается, но нельзя работать с одним экземпляром из разных потоков.
 * Поддерживается pthread_cancel().
 * 
 * При возникновении исключительной ситуации (assertion failed)
 * выдаётся исключение (по умолчанию) или завершается процесс (см. UNP::LibraryInit()).
 * 
 * Поддерживается как бинарный, так и текстовый протокол.
 * По умолчанию используется бинарный.
 * На текстовый сервер переключается автоматически,
 * если первый байт запроса является символом ASCII.
 * Текстовый протокол удобен для взаимодействия с сервером из консоли (например, через telnet или ncat).
 * 
 * Описание текстового протокола с примером приведено в конце данного файла.
 * 
 *  Created on: 31.01.2014
 *      Author: kudr
 */

#ifndef UNP_RPC_H_
#define UNP_RPC_H_

#include <inttypes.h>
#include <string>

namespace UNP {

class RPC_Message;

//! Аргументы
class RPC_Args {
public:
	RPC_Args();
	~RPC_Args();
	
	RPC_Args(const RPC_Args &) = delete;
	RPC_Args& operator=(const RPC_Args &) = delete;
	
	//! \name Формирование аргументов
	//! {
	RPC_Args& operator << (int32_t);
	RPC_Args& operator << (uint32_t);
	RPC_Args& operator << (int64_t);
	RPC_Args& operator << (uint64_t);
	RPC_Args& operator << (double);
	RPC_Args& operator << (const std::string &);
	RPC_Args& operator << (const char*); // копируется содержимое
	//! }
	
	/*! \name Разбор аргументов.
	 * При ошибке вернёт значение по умолчанию и выведет в лог соответствующее сообщение.
	 * При ошибке разбор останавливается.
	 */
	//! {
	RPC_Args& operator >> (int32_t &);
	RPC_Args& operator >> (uint32_t &);
	RPC_Args& operator >> (int64_t &);
	RPC_Args& operator >> (uint64_t &);
	RPC_Args& operator >> (double &);
	RPC_Args& operator >> (std::string &);
	RPC_Args& operator >> (const char* &); // копируется указатель на данные в буфере внутри RPC_Args
	//! }
	
	void Clear();
	
	/*! \name Получение признака ошибки формирования аргументов (<<)
	 * @return
	 */
	inline bool FormError() const { return false; }
	
	/*! \name Получение признака ошибки разбора аргументов (>>)
	 * @return
	 */
	inline bool ParseError() const { return parseError; }
	
private:
	RPC_Message *message;
	bool parseError;
	
	friend class RPC_ClientImpl;
	friend void* RPC_Server_CmdHandle_th(void*);
};

}
#endif

/*
 * Текстовый протокол.
 * 
 * Сообщение состоит из набора значений.
 * В качестве разделителя значений в запросе можно использовать любой пробельный символ (пробел, табуляция и т.д.).
 * В ответе сервер в качестве разделителя использует табуляцию.
 * Признак конца сообщения - любой из символов '\n', '\r', '\0'.
 * Значения типа "строка" можно взять в кавычки,
 * тогда внутри строки можно будет использовать пробелы
 * и экранировать символы при помощи \
 * (использование символов конца сообщения в значениях не поддерживается).
 * 
 * Поддерживается только полудуплекс: запрос, ответ, запрос, ответ и т.д.
 * Отправка нескольких запросов подряд без ожидания соответствующих ответов не поддерживается.
 * 
 * Пример.
 * 
 * Запустим сервер из примера:
 *     ./rpc_server 
 *     Server: TCP: 0.0.0.0:10000
 *     Attempt to listen '0.0.0.0:10000'
 *     Listen begin
 * 
 * Сервер слушает TCP порт 10000.
 * 
 * Подключимся к нему через telnet:
 *     telnet 127.0.0.1 10000
 *     Trying 127.0.0.1...
 *     Connected to 127.0.0.1.
 *     Escape character is '^]'.
 * 
 * Отправим команду 1 (вычисление суммы двух целых чисел):
 *     1 12 25 <Enter>
 * Ответ:
 *     37
 * 
 * Полный список поддерживаемых команд тестовым сервером можно посмотреть в rpc_server.cc.
 */
