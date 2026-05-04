/*
 * unp_time.h
 *
 * Время
 *
 *  Created on: 21.08.2015
 *      Author: kudr
 */

#ifndef UNP_TIME_H_
#define UNP_TIME_H_

#include <inttypes.h>

namespace UNP {

//! Тип источника времени
enum TimeType {
	TT_REAL = 0,     //!< Стандартное время.
	                 //!< Источник времени основан на системных часах.
	
	TT_MONOTONIC,    //!< Монотонное время.
	                 //!< Начинается с 0 и не убывает.
	
#ifdef __QNX__ 
	TT_CLOCK_CYCLES, //!< Время, основанное на счётчике тактов процессора.
	                 //!< Корректно работает только для потоков, привязанных к конкретному процессору.
	                 //!< Переполнение счётчика не учитывается.
#endif
};

//! Единицы измерения времени
enum TimeMeasure {
	TM_S = 0, //!< Секунды
	TM_MS,    //!< Миллисекунды
	TM_US,    //!< Микросекунды
	TM_NS     //!< Наносекунды
};

/*!
 * Получение времени
 * 
 * @param type    Тип
 * @param measure Единицы измерения
 * 
 * @return Время в нужных единицах измерения
 */
int64_t GetTime(TimeType type, TimeMeasure measure);

static inline int64_t GetTime() {
	return GetTime(TT_MONOTONIC, TM_MS);
}

static inline int64_t GetTime(TimeMeasure measure) {
	return GetTime(TT_MONOTONIC, measure);
}

/*!
 * В конструкторе сохраняет текущее время,
 * в деструкторе вычисляет прошедшее и печатает на std.
 */
class TimeElapsed {
	int64_t t;
	
public:
	TimeElapsed();
	~TimeElapsed();
};

}
#endif
