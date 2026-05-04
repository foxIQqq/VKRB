/*
 * unp_bpf.h
 *
 *  Created on: 14 сент. 2023 г.
 *      Author: kudr
 */

#ifndef INC_UNP_BPF_H_
#define INC_UNP_BPF_H_

#ifdef __QNXNTO__

#include <inttypes.h>

namespace UNP {

//! Приём через BPF (UDP)
class BpfRecv {
public:
	BpfRecv();
	virtual ~BpfRecv();
	
	BpfRecv(const BpfRecv&) = delete;
	BpfRecv& operator=(const BpfRecv&) = delete;
	
	/*!
	 * Открыть
	 * 
	 * @param ifName         Имя сетевого интерфейса (именно имя, не адрес)
	 * @param filterAddress  IP-адрес фильтра
	 * @param filterPort     Порт фильтра
	 * @param rcvBufLen      Желаемый размер приёмного буфера
	 * @param immediate      
	 * 
	 * @return "Успех" / "ошибка"
	 */
	bool Open(const char *ifName, const char *filterAddress, unsigned filterPort, unsigned rcvBufLen, bool immediate = true);
	
	//! Закрыть
	void Close();
	
	inline bool IsOpened() const { return isOpened; }
	
	/*!
	 * Чтение.
	 * 
	 * За одно чтение может быть считано несколько датаграмм.
	 * Для обработки всех датаграмм нужно вызывать Parse(), пока он возвращает true.
	 */
	bool Read();
	
	/*!
	 * Разбор прочитанных данных
	 * 
	 * @param pdatagram  Сюда будет записан адрес буфера с датаграммой (сам буфер находится внутри этого класса) (не NULL)
	 * @param plen       Сюда будет записан размер датаграммы (не NULL)
	 * 
	 * @return true означает, что есть ещё датаграммы
	 */
	bool Parse(uint8_t **pdatagram, unsigned *plen);
	
private:
	int fd;
	int fdExtra; // для IGMP, ICMP
	
	bool isOpened;
	
	uint8_t *rcvBuf;
	unsigned rcvBufAllocated;
	int n, offset;
	
	void ReallocRcvBuf(unsigned rcvBufLen);
};

//! \name Вспомогательные ф-ии
//! {

/*!
 * Загрузка фильтра
 * 
 * @param fd    Дескриптор (/dev/bpf)
 * @param addr  IP адрес
 * @param port  Порт
 * 
 * @return "Успех" / "ошибка"
 */
bool BpfLoadFilter(int fd, const char *addr, unsigned port);

//! }

}
#endif
#endif /* INC_UNP_BPF_H_ */
