/*
 * unp_serial.h
 *
 * Работа с последовательным портом
 * (pthread_cancel() поддерживается).
 *
 * При ошибке бросается исключение типа int:
 *  - код ошибки из errno.h;
 *    специальные исключения:
 *      ETIMEDOUT - таймаут
 *      ENODATA   - EOF (read() вернул 0)
 *  - 0 - просто ошибка без уточнения
 *
 *  Created on: 22 мар. 2023 г.
 *      Author: kudr
 */

#ifndef UNP_SERIAL_H_
#define UNP_SERIAL_H_

#include <inttypes.h>

namespace UNP {

enum SerialParity {
	SERIAL_PARITY_NONE = 0, //!< Отключен контроль чётности
	SERIAL_PARITY_EVEN,     //!< Чёт.
	SERIAL_PARITY_ODD,      //!< Нечёт.
};

//! Настройки последовательного порта
struct SerialSets {
	int iSpeed;
	int oSpeed;
	int dataBits; //!< от 5 до 8 (потенциально можно поддержать и 9)
	int stopBits; //!< 1 или 2
	SerialParity parity = SERIAL_PARITY_NONE; //!< Контроль чётности
	
	SerialSets(int speed_, int dataBits_, int stopBits_, SerialParity parity_)
		: iSpeed(speed_), oSpeed(speed_), dataBits(dataBits_), stopBits(stopBits_), parity(parity_)
	{}
	
	SerialSets(int iSpeed_, int oSpeed_, int dataBits_, int stopBits_, SerialParity parity_)
		: iSpeed(iSpeed_), oSpeed(oSpeed_), dataBits(dataBits_), stopBits(stopBits_), parity(parity_)
	{}
};

class SerialPort {
	int fd = -1;
	bool needPause = false, wasTransmited = false;
	int64_t tLastTransmit_us = 0; // время (после tcdrain!)
	int speedMax = 0;
	bool verbose = false;
	uint8_t termCur[160]{};
	
public:
	SerialPort(bool verbose_ = false) : verbose(verbose_) {}
	virtual ~SerialPort();
	
	SerialPort(const SerialPort&) = delete;
	void operator=(const SerialPort&) = delete;
	
	void Open(const char *path, const SerialSets &sets, bool nonBlock = false);
	void Close();
	
	inline bool IsOpened() const { return fd >= 0; }
	
	void ChangeSerialSets(const SerialSets &sets);
	
	/*!
	 * Чтение не более заданного количества байт.
	 * 
	 * @param buf   Буфер
	 * @param size  Максимальное количество байт для чтения
	 * 
	 * @param timeout_ms  Таймаут (мс):
	 *                       > 0 - ожидание данных не более заданного времени;
	 *                       -1  - время ожидания не ограничено;
	 *                       0   - не ждать
	 * 
	 * @param logTo        Логировать таймаут
	 * 
	 * @return  Прочитанное кол-во байт (гарантированно >0 при size >0 (ошибки - через исключения))
	 */
	size_t Read(void *buf, size_t size, int timeout_ms, bool logTo = true);
	
	/*!
	 * Чтение ровно n байт.
	 * 
	 * @param buf              Буфер
	 * @param n                Количество байт для чтения
	 * @param timeoutFirst_ms  Таймаут (мс) первого байта (см. Read())
	 * @param timeoutNext_ms   Таймаут (мс) для всех последующих байт (см. Read())
	 * @param logToFirst       Логировать таймаут первого байта
	 * @param logToNext        Логировать таймаут для всех последующих байт
	 */
	void ReadN(void *buf, size_t n, int timeoutFirst_ms, int timeoutNext_ms, bool logToFirst = true, bool logToNext = true);
	
	/*!
	 * Запись ровно n байт
	 */
	void WriteN(const void *buf, size_t n);
	
	//! Удалить все буферизированные данные
	void Discard();
	
	//! Дождаться отправки буферизированных данных
	void Drain();
	
	inline int GetFd() { return fd; }
	inline void SetVerbose(bool v) { verbose = v; }
	
private:
	void SleepIfNeed();
};

}
#endif
