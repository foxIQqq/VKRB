/*
 * unp_prod_cons.h
 *
 * Производитель-потребитель.
 * 
 * Здесь только вспомогательный функционал -
 * без ожиданий, синхронизации доступа и т.д.
 *
 *  Created on: 11.10.2016
 *      Author: kudr
 */

#ifndef UNP_PROD_CONS_H_
#define UNP_PROD_CONS_H_

#include <queue>

namespace UNP {
namespace PROD_CONS {

class Task {
public:
	virtual ~Task() {}
};

class Fifo {
	std::queue<Task*> vacant;  //!< Свободные элементы
	std::queue<Task*> fifo;    //!< Очередь на обработку потребителем
	Task *prod;                //!< Элемент, полученный производителем
	Task *cons;                //!< Элемент, полученный потребителем
	
public:
	/*!
	 * Конструктор
	 * @param pool Пул (элементы д.б. не NULL, будут удалены в деструкторе)
	 */
	Fifo(std::queue<Task*> &pool) : prod(NULL), cons(NULL) { vacant.swap(pool); }
	
	virtual ~Fifo();
	
	Fifo(const Fifo&) = delete;
	void operator=(const Fifo&) = delete;
	
	//! Переместить все эл-ы из очереди на обработку в список свободных (полученные производителем и потребителем остаются)
	void VacateFifo();
	
	//! Переместить все эл-ты (из очереди на обработку, полученные производителем и потребителем) в список свободных
	void VacateAll();
	
	//! Проверка на то, что очередь на обработку пуста
//	bool IsFifoEmpty() const { return fifo.empty(); }
	
	//! Проверка на то, что все элементы свободны
//	bool IsAllVacant() const { return fifo.empty() && !prod && !cons; }
	
	//! Проверка на то, что потребитель всё обработал
	bool IsAllConsumed() const { return fifo.empty() && !cons; }
	
	//! \name Производитель
	//! {
	
	//! Получение свободного эл-та
	Task* ProducerTake();
	
	//! Помещение полученного эл-та в очередь на обработку (toProcessFifo true) или в список свободных (false)
	void ProducerPut(bool toProcessFifo = true);
	
	//! }
	
	//! \name Потребитель
	//! {
	
	//! Получение эл-та на обработку
	Task* ConsumerTake();
	
	//! Помещение полученного эл-та в список свободных
	void ConsumerPut();
	
	//! }
};

}
}
#endif
