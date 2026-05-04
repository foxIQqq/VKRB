/*
 * unp_phys_buf.h
 *
 * Буфер в непрерывной физической памяти
 *
 *  Created on: 12.04.2016
 *      Author: kudr
 */

#ifndef UNP_PHYS_BUF_H_
#define UNP_PHYS_BUF_H_

#ifdef __QNXNTO__

#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

namespace UNP {

//! Буфер в непрерывной физической памяти
class PhysBuf {
public:
	uint8_t *buf;
	size_t size;
	off64_t physAddr;
	
public:
	PhysBuf();
	virtual ~PhysBuf();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * @param size     Размер буфера (в байтах)
	 * @param noCache  Отключение кэширования процессором
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(size_t size, bool noCache = false);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
private:
	PhysBuf(const PhysBuf&);
	PhysBuf& operator=(const PhysBuf&);
};

/*!
 * Буфер в разделяемой непрерывной физической памяти.
 * 
 * Отличие от PhysBuf в том, что память (физические адреса, как следствие)
 * не будет освобождена при завершении работы с объектом,
 * и при повторном использовании будем работать с теми же физическими адресами.
 */
class PhysBufShared {
	int shmFd;
	
public:
	uint8_t *buf;
	size_t size;
	off64_t physAddr;
	
public:
	PhysBufShared();
	virtual ~PhysBufShared();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * @param name     Имя объекта разделяемой памяти (для совместимости должно начинаться с '/')
	 * @param size     Размер буфера (в байтах)
	 * @param noCache  Отключение кэширования процессором
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(const char *name, size_t size, bool noCache = false);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
private:
	PhysBufShared(const PhysBufShared&);
	PhysBufShared& operator=(const PhysBufShared&);
};

/*!
 * Работа с памятью устройства по физическому адресу.
 */
class PhysBufDev {
public:
	off64_t paddr; 
	volatile uint8_t *vaddr; //!< Виртуальный адрес. При работе с памятью процессора volatile не нужен.
	size_t size;
	bool noCache;
	
public:
	PhysBufDev();
	virtual ~PhysBufDev();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * @param paddr  Физический адрес
	 * @param size   Размер (в байтах)
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(off64_t paddr, size_t size, bool noCache);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
private:
	PhysBufDev(const PhysBufDev&);
	PhysBufDev& operator=(const PhysBufDev&);
};

}
#endif
#endif
