/*
 * unp_sock.h
 * 
 * Работа с сокетами
 * (pthread_cancel() поддерживается).
 * 
 * Все функции при ошибке устанавливают errno.
 * При неполной передаче / приёме возвращается EBADE.
 * При неожиданном конце файла - ENODATA.
 * 
 * Функции создания сокета возвращают дескриптор сокета или отрицательное значение при ошибке.
 * 
 * В качестве сетевого интерфейса (аргументы iface)
 * можно передавать как его имя, так и адрес.
 * Если значение начинается с цифры - считаем, что это адрес; иначе - имя.
 * 
 * Сокеты в этой библиотеке также являются сокетами POSIX. 
 * 
 *  Created on: 04.12.2013
 *      Author: kudr
 */

#ifndef UNP_SOCK_H_
#define UNP_SOCK_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace UNP {

/*!
 * Закрытие сокета
 * 
 * @param sock
 * @param localStreamListenPath
 */
void Sock_Close(int sock, const char *localStreamListenPath = NULL);

/*!
 * Приём
 * @param sock
 * @param buf
 * @param n
 * @param logEof        Признак логирования ошибки "Конец файла" (ENODATA)
 * @param logTimeout    Признак логирования ошибки "Таймаут" (EAGAIN)
 * @param logConnReset  Признак логирования ошибки "Клиент сбросил соединение" (ECONNRESET)
 * @return Количество принятых байт; <0 - ошибка
 */
ssize_t Sock_Read(int sock, void *buf, size_t n, bool logEof = true, bool logTimeout = true, bool logConnReset = true);

/*!
 * Приём ровно n байт
 * @param sock
 * @param buf
 * @param n
 * @param logEof        Признак логирования ошибки "Конец файла" (ENODATA)
 * @param logTimeout    Признак логирования ошибки "Таймаут" (EAGAIN)
 * @param logConnReset  Признак логирования ошибки "Клиент сбросил соединение" (ECONNRESET) 
 * @return  Успех / ошибка
 */
bool Sock_ReadN(int sock, void *buf, size_t n, bool logEof = true, bool logTimeout = true, bool logConnReset = true);

/*!
 * Векторный вариант Sock_ReadN()
 * @param sock
 * @param iov
 * @param iovCnt
 * @param totalSize   Суммарное количество байт, используется только для проверки
 * @param logEof      Признак логирования ошибки "Конец файла" (ENODATA)
 * @param logTimeout  Признак логирования ошибки "Таймаут" (EAGAIN)
 * @return  Успех / ошибка
 */
bool Sock_ReadNV(int sock, iovec *iov, int iovCnt, size_t totalSize, bool logEof = true, bool logTimeout = true);

/*!
 * Отправка ровно n байт
 * @param sock
 * @param buf
 * @param n
 * @return  Успех / ошибка
 */
bool Sock_WriteN(int sock, const void *buf, size_t n);

/*!
 * Векторный вариант Sock_WriteN()
 * @param sock
 * @param iov
 * @param iovCnt
 * @param totalSize  Суммарное количество байт, используется только для проверки
 * @return  Успех / ошибка
 */
bool Sock_WriteNV(int sock, const iovec *iov, int iovCnt, size_t totalSize);

/*!
 * Установка таймаутов
 * @param sock
 * @param timeout_msec  Таймаут (мс; <0 - не устанавливать, 0 - без ограничения по времени)
 * @param target        1 - таймаут на отправку, 2 - на приём, 3 - оба
 * @return  Успех / ошибка
 */
bool Sock_SetTimeouts(int sock, int timeout_msec, unsigned target = 3);

/*!
 * Настройка сокета
 * @param sock
 * @param level
 * @param optNameS
 * @param optName
 * @param optVal
 * @param optLen
 * @return  Успех / ошибка
 */
bool Sock_SetOpt(int sock, int level, const char *optNameS, int optName, const void * optVal, socklen_t optLen);

/*!
 * Создание сокета TCP и соединение с указанием ip-адреса и порта
 * @param address
 * @param port
 * @param timeout_ms  Таймаут (мс; <0 - не устанавливать, 0 - именно 0)
 * @return Сокет
 */
int SockTCP_Connect(const char *address, unsigned port, int timeout_ms);

/*!
 * Создание сокета TCP и соединение с указанием имени узла и имени службы
 * @param nodeName    Имя узла ("smtp.mastermail.ru") или ip-адрес ("90.156.155.143")
 * @param servName    Имя службы или номер порта
 * @param timeout_ms  Таймаут на подключение (без учёта разрешения адреса) (мс; <0 - не устанавливать, 0 - именно 0)
 * @return Сокет
 */
int SockTCP_Connect_names(const char *nodeName, const char *servName, int timeout_ms);

/*!
 * Создание прослушивающего TCP сокета
 * @param address         Адрес; NULL или пустая строка => все адреса
 * @param port
 * @param listenQueueLen
 * @return Сокет
 */
int SockTCP_Listen(const char *address, unsigned port, int listenQueueLen = 10);

/*!
 * Ожидание входящего подключения TCP
 * @param sockListen
 * @return Сокет
 */
int SockTCP_Accept(int sockListen);

/*!
 * Просто создание (без подключения и т.д.) сокета TCP
 * @return
 */
int SockTCP_Simple();

/*!
 * Подключение через локальный потоковый сокет
 * @param path
 * @param timeout_ms  Таймаут (мс; <0 - не ограничено)
 * @return Сокет
 */
int SockLocalStream_Connect(const char *path, int timeout_ms);

/*!
 * Создание прослушивающего локального потокового сокета
 * @param path
 * @param listenQueueLen
 * @return Сокет
 */
int SockLocalStream_Listen(const char *path, int listenQueueLen = 10);

/*!
 * Ожидание входящего подключения по локальному потоковому сокету
 * @param sockListen
 * @return Сокет
 */
int SockLocalStream_Accept(int sockListen);

/*!
 * Просто создание (без подключения и т.д.) локального потокового сокета
 * @return Сокет
 */
int SockLocalStream_Simple();

/*!
 * Определение адреса сетевого интерфейса
 * 
 * @param pres Результирующий адрес (не NULL)
 * 
 * @param iface Имя или адрес сетевого интерфейса.
 *              Если NULL, то вернёт INADDR_ANY.
 *              Если начинается с цифры, то считаем, что ifname - это адрес, и возвращаем его.
 *              Иначе определяем по имени.
 *
 * @param cycle Многократные попытки определения адреса до победного / однократная попытка
 *  
 * @return  Успех / ошибка
 */
bool Sock_GetIfAddress(in_addr_t *pres, const char *iface, bool cycle);

bool Sock_IsMulticastAddress(in_addr_t address);
bool Sock_IsMulticastAddress(const char *address);

/*!
 * Создание соединённого сокета UDP
 * @param address
 * @param port
 * @param iface      Имя или адрес сетевого интерфейса (только для многоадресной передачи)
 * @param loopEnable Разрешить передачу по обратной петле (только для многоадресной передачи)
 * @return Сокет
 */
int SockUDP_Connect(const char *address, unsigned port, const char *iface, bool loopEnable);

/*!
 * Настройка TTL
 * @param sock
 * @param ttl
 * @return  Успех / ошибка
 */
bool Sock_SetTtl(int sock, int ttl);

/*!
 * Настройка TTL для многоадресной передачи
 * @param sock
 * @param ttl
 * @return  Успех / ошибка
 */
bool Sock_SetMulticastTtl(int sock, int ttl);

/*!
 * Создание прослушивающего UDP сокета
 * @param address Адрес; NULL или пустая строка => все адреса
 * @param port
 * @param iface   Имя или адрес сетевого интерфейса (только для многоадресной передачи)
 * @return Сокет
 */
int SockUDP_Listen(const char *address, unsigned port, const char *iface);

/*!
 * Простое создание (без подключения и т.д.) сокета UDP
 * @return Сокет
 */
int SockUDP_Simple();

/*!
 * Присоединение к группе многоадресной передачи.
 * Используется при приёме.
 * Функция вспомогательная, и при использовании SockUDPListen_Open() вызывать её не нужно.
 * 
 * @param sock
 * @param groupAddress
 * @param groupPort
 * @param iface
 * 
 * @return  Успех / ошибка
 */
bool Sock_MulticastJoin(int sock, in_addr_t groupAddress, unsigned groupPort, const char *iface);

/*!
 * Установить интерфейс для исходящих пакетов при многоадресной передачи.
 * Функция вспомогательная, и при использовании SockUDP_Connect() вызывать её не нужно.
 * 
 * @param sock
 * @param iface
 * 
 * @return  Успех / ошибка
 */
bool Sock_MulticastSetIface(int sock, const char *iface);

#ifdef __QNXNTO__
/*!
 * Определение статуса линка сетевого интерфейса
 *
 * @param sock   Дескриптор сокета 
 * @param ifName Имя сетевого интерфейса
 * 
 * @return
 *   0  - нет линка
 *   >0 - есть
 *   <0 - ошибка
 */
int Sock_GetIfLinkStatus(int sock, const char *ifName);
#endif

}
#endif
