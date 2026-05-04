/*
 * unp_irq.h
 *
 * Работа с прерываниями
 *
 *  Created on: 19.04.2016
 *      Author: kudr
 */

#ifndef UNP_IRQ_H_
#define UNP_IRQ_H_

#ifdef __QNXNTO__

#include <inttypes.h>
#include <sys/siginfo.h>

namespace UNP {

//! Обработчик прерывания
//! (процесс должен быть запущен от root и предварительно вызвать ThreadCtl(_NTO_TCTL_IO, 0))
class Irq {
protected:
	int intId;
	sigevent isrEvent;
	
	int irq;
	bool verbose;
	
private:
	int chId;
	int coId;
	
	bool intMasked;
	
	int64_t intClearT_us;
	int64_t intClearDtMin_us;
		
public:
	/*!
	 * Конструктор
	 * @param verbose  Логирование через trace_logf()
	 */
	Irq(bool verbose = false);
	
	virtual ~Irq();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * 
	 * @param irq          IRQ
	 * @param pulsePrio    Приоритет импульса
	 * @param classSizeof  sizeof() класса-наследника Irq
	 * 
	 * @param flagEnd      _NTO_INTR_FLAGS_END
	 *                     Put the new handler at the end of the list of existing handlers (for shared interrupts) instead of the start.
	 *                     
	 * @param flagProcess  _NTO_INTR_FLAGS_PROCESS
	 *                     Associate the handler with the process instead of the attaching thread.
	 * 
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(int irq, int pulsePrio = SIGEV_PULSE_PRIO_INHERIT, int classSizeof = sizeof(Irq), bool flagEnd = false, bool flagProcess = false);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
	/*!
	 * Ожидание прерывания без таймаута
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Wait();
	
	/*!
	 * Ожидание прерывания с таймаутом
	 * @param timeout_ns  Таймаут (нс)
	 * @param isTimeout   Если не NULL, то сюда при таймауте будет записано true, в противном случае - false
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка (таймаут также считается "ошибкой")
	 */
	bool WaitTimeout(uint64_t timeout_ns, bool *isTimeout);
	
	inline int GetIrq() const { return irq; }
	
	/*!
	 * Уведомление о сбросе прерывания
	 * @param dtMinClearUnmask_us  Минимальное время между сбросом прерывания и демаскированием
	 *                             (нужно для того, чтобы не получить "старое" прерывание)
	 */
	void SetIntCleared(int64_t dtMinClearUnmask_us = 10);
	
	/*!
	 * Демаскирование прерывания
	 * (вызывать не обязательно, т.к. прерывание автоматически демаскируется в Wait*())
	 */
	void Unmask();
	
protected:
	/*!
	 * Низкоуровневый обработчик прерывания.
	 * Реализация по умолчанию маскирует прерывание и возвращает &isrEvent.
	 * @return
	 */
	virtual const struct sigevent* Isr();
	
	/*!
	 * Класс-потомок должен указать, маскирует ли он IRQ в ISR
	 * @return
	 */
	virtual bool MaskedInIsr() { return true; }
	
private:
	static const struct sigevent* Isr_st(void *arg, int id);
	
	void IntDelay();
};

}
#endif
#endif
