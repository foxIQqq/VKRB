/*
 * unp_threads.h
 *
 * Потоки.
 * 
 * Функции и методы, возвращающие void, в случае ошибки завершают процесс
 * (предполагается, что в нормальной работе используемые вызовы pthread_*() должны завершаться без ошибок).
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_THREADS_H_
#define UNP_THREADS_H_

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>

#include <string>

#include "unp_assert.h"

//-V:pthread_cleanup_push:506

//! Размер стека по умолчанию для потока
#define UNP_THREAD_STACK_SIZE_DEFAULT 131072

namespace UNP {

/*!
 * Создание потока
 * 
 * @param ptid      Если не NULL, то сюда будет помещён идентификатор потока, созданый присоединяемым;
 *                  в противном случае поток создаётся обособленным
 * @param rtn       Функция потока
 * @param rtnArg    Аргумент, передаваемый в функцию потока
 * @param stackSize Размер стека (в байтах; значение 0 означает, что размер стека определяется системой)
 *  
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
bool ThreadCreate(pthread_t *tid, void *(*rtn)(void *), void *rtnArg, size_t stackSize = UNP_THREAD_STACK_SIZE_DEFAULT);

void SetCancelState(int state, int *oldstate = NULL);

//! Мьютекс
class Mutex {
public:
	Mutex();
	virtual ~Mutex();
	
	/*!
	 * Захват мьютекса.
	 * Точка отмены: нет.
	 */
	void Lock();
	
	/*!
	 * Попытка захвата мьютекса без блокирования потока.
	 * Точка отмены: нет.
	 * 
	 * @return "Истина" - удалось захватить мьютекс
     *         "Ложь"   - не удалось захватить мьютекс
	 */
	bool Trylock();
	
	/*!
	 * Освобождение мьютекса
	 */
	void Unlock();
	
	/*!
	 * Вызов ->Unlock()
	 * @param thiz  Указатель на UNP::Mutex.
	 *              При множественном наследовании нужно явно приводить к UNP::Mutex* .
	 */
	static void UnlockSt(void *thiz); 
	
protected:
	pthread_mutex_t mutex;
	
private:
	Mutex(const Mutex &);
	Mutex &operator=(const Mutex &);
};

//! Мьютекс и переменная состояния
class CondMutex : public Mutex {
public:
	typedef bool (*Condition_f)(void *userData);
	
	CondMutex();
	virtual ~CondMutex();
	
	/*!
	 * Ожидание переменной состояния.
	 * Точка отмены: да.
	 */
	void Wait();
	
	/*!
	 * Ожидание переменной с ограничением по времени.
	 * Точка отмены: да.
	 * 
	 * @param absTime_mono  Абсолютное время (используется монотонный источник времени (CLOCK_MONOTONIC))
	 * @param isTimeout     Сюда (указатель должен быть не NULL) при таймауте будет записано true, в противном случае - false
	 */
	void WaitTimed(const timespec &absTime_mono, bool *isTimeout);
	
	/*!
	 * Ожидание в цикле, пока условие истинно.
	 * В начале мьютекс блокируется, в конце - разблокируется (опционально).
	 * Точка отмены: да.
	 * 
	 * @param userData      Пользовательские данные, передаваемые в *fun 
	 * @param conditionFun  Функция, вызываемая для проверки условия (не NULL)
	 * @param needUnlock    Разблокировать в конце
	 */
	void WaitCycle(void *userData, Condition_f conditionFun, bool needUnlock = true);
	
	/*!
	 * Ожидание в цикле, пока выражение истинно, с ограничением по времени.
	 * В начале мьютекс блокируется, в конце - разблокируется (опционально).
	 * Точка отмены: да.
	 * 
	 * @param userData      Пользовательские данные, передаваемые в *fun 
	 * @param conditionFun  Функция, вызываемая для проверки условия (не NULL)
	 * @param needUnlock    Разблокировать в конце
	 */
	void WaitCycleTimed(uint64_t timeout_ns, bool *isTimeout, void *userData, Condition_f conditionFun, bool needUnlock = true);
	
	/*!
	 * Аналогично WaitCycleTimed(), но точкой отмены не является
	 */
	void WaitCycleTimed_nc(uint64_t timeout_ns, bool *isTimeout, void *userData, Condition_f conditionFun, bool needUnlock = true);
	
	/*!
	 * Разблокирование ожидающих потоков.
	 * Точка отмены: нет.
	 */
	void Trigger();
	
protected:
	pthread_cond_t cond;
};

//! Блокировка чтения-записи
class Rwlock {
public:
	Rwlock();
	virtual ~Rwlock();
	
	/*!
	 * Захват блокировки на чтение.
	 * Точка отмены: нет.
	 */
	void Rdlock();
	
	/*!
	 * Захват блокировки на запись.
	 * Точка отмены: нет.
	 */
	void Wrlock();
	
	/*!
	 * Освобождение мьютекса
	 */
	void Unlock();
	
protected:
	pthread_rwlock_t rwlock;
};

//! Включение режима PTHREAD_CANCEL_DISABLE в конструкторе и восстановление предыдущего значения в деструкторе
class ThreadCancelDisabler {
public:
	ThreadCancelDisabler() { SetCancelState(PTHREAD_CANCEL_DISABLE, &oldCancelState); }
	virtual ~ThreadCancelDisabler() { SetCancelState(oldCancelState, NULL); }
	
private:
	int oldCancelState;
	
	ThreadCancelDisabler(const ThreadCancelDisabler &);
	ThreadCancelDisabler &operator=(const ThreadCancelDisabler &);
};

//! Блокировка мьютекса в конструкторе и разбокировка в деструкторе
class MutexLocker {
public:
	MutexLocker(Mutex &mutex_) : mutex(&mutex_) { mutex->Lock(); }
	virtual ~MutexLocker() { mutex->Unlock(); }
	
private:
	Mutex *mutex;
	
	MutexLocker(const MutexLocker &);
	MutexLocker &operator=(const MutexLocker &);
};

//! Семафор
class Sem {
public:
	/*!
	 * Создание неименованного семафора
	 * @param value Начальное значение семафора
	 */
	Sem(unsigned value);
	
	virtual ~Sem();
	
	/*!
	 * Ожидание.
	 * Точка отмены: да.
	 */
	void Wait();
	
#ifdef __QNXNTO__
	/*!
	 * Ожидание с ограничением по времени.
	 * Точка отмены: да.
	 * 
	 * @param absTime_mono  Абсолютное время (используется монотонный источник времени (CLOCK_MONOTONIC))
	 * @param isTimeout     Сюда (указатель должен быть не NULL) при таймауте будет записано true, в противном случае - false
	 */
	void WaitTimed(const timespec &absTime_mono, bool *isTimeout);
#endif
	
	/*!
	 * Инкремент
	 */
	void Post();
	
private:
	sem_t sem;
	
	Sem(const Sem &);
	Sem &operator=(const Sem &);
};

#ifdef __QNXNTO__
//! Таймер
class Timer {
public:
	Timer();
	virtual ~Timer();
	
	/*!
	 * Установка таймера
	 * 
	 * @param value_ns     Время срабатывания (нс; 0 - выкл.)
	 * @param interval_ns  Период (нс; 0 - без повторов)
	 * @param isAbs        Абсолютное / относительное время
	 */
	void SetTime(int64_t value_ns, int64_t interval_ns = 0, bool isAbs = false);
	
	/*!
	 * Ожидание срабатывания
	 * Точка отмены: да
	 */
	void Wait();
	
private:
	sigevent event;
	timer_t timerId;
	bool timerCreated;
	int chId;
	
	Timer(const Timer &);
	Timer &operator=(const Timer &);
	
	void Release();
};
#endif

/*!
 * Персональные данные потока.
 * 
 * Экземпляр должен быть создан до Run_th() и передан туда в качестве аргумента.
 * Удаляется автоматически при завершении потока.
 * 
 * Инициализация предполагается в Init().
 * Деинициализация - в деструкторе.
 * В этих методах об отмене потока (pthread_cancel()) можно не беспокоиться -
 * перед вызовом Init() устанавливается PTHREAD_CANCEL_DISABLE (после восстанавливается предыдущее значение),
 * а деструктор вызывается из обработчика завершения потока (pthread_cleanup_push()), где отмена не актуальна. 
 */
class ThreadInfo {
	std::string name;
	bool quiet;
	
public:
	static void SetErrorHandler(pthread_t tid, int sigNumber);
	static pthread_t GetErrorHandlerTid();
	static int GetErrorHandlerSigNumber();
	
	/*!
	 * Конструктор
	 * @param name  Имя потока
	 * @param quiet Отключить логирование (в конструкторе и деструкторе)
	 */
	ThreadInfo(const char *name, bool quiet = false);
	ThreadInfo(const std::string &name, bool quiet = false);
	
	virtual ~ThreadInfo();
	
	static void* Run_th(ThreadInfo *thiz);
	
protected:
	virtual bool Init() = 0;
	virtual bool Process() = 0;
	
	inline const char* GetName() const { return name.c_str(); }
	
private:
	ThreadInfo(const ThreadInfo &);
	ThreadInfo &operator=(const ThreadInfo &);
	
	static void Delete(void* thiz);
};

/*!
 * Запустить поток.
 * 
 * Если *ptid == 0, то создаётся поток, а в *ptid будет записан его pthread_t;
 * иначе - просто вернёт true.
 * 
 * @param ptid      Указатель на pthread_t (не NULL)
 * @param rtn
 * @param rtnArg
 * @param stackSize
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
static inline bool ThreadStart(pthread_t *ptid, void *(*rtn)(void *), void *rtnArg = NULL, size_t stackSize = UNP_THREAD_STACK_SIZE_DEFAULT) { // p* - не NULL
	if(*ptid)
		return true;
	
	return UNP::ThreadCreate(ptid, rtn, rtnArg, stackSize);
}

/*!
 * Остановить поток.
 * 
 * Если *ptid != 0, то останавливается поток (pthread_cancel() и pthread_join()), а в *ptid будет записан 0;
 * иначе - просто возврат.
 * 
 * @param ptid Указатель на pthread_t (не NULL)
 */
static inline void ThreadStop(pthread_t *ptid) {
	if(*ptid) {
		UNP::ThreadCancelDisabler d;
		
		pthread_cancel(*ptid);
		pthread_join(*ptid, NULL);
		*ptid = 0;
	}
}

#ifndef __QNXNTO__
static inline void nsec2timespec(struct timespec *ts, uint64_t nsec) {
	static_assert(sizeof(ts->tv_sec) + sizeof(ts->tv_nsec) == sizeof(*ts), "");
	ts->tv_sec = nsec / 1000000000;
	ts->tv_nsec = nsec % 1000000000;
}
static inline uint64_t timespec2nsec(const struct timespec *ts) {
	return ts->tv_sec * (uint64_t)1000000000 + ts->tv_nsec;
}
#endif

//! Получить timespec (монотонное время)
static inline void GetTimeSpec(timespec *t, uint64_t timeout_ns) {
	int r = clock_gettime(CLOCK_MONOTONIC, t);
	UNP::AssertSysError(r == 0, errno, "clock_gettime");
	
	nsec2timespec(t, timespec2nsec(t) + timeout_ns);
}

#ifdef __QNXNTO__
/*!
 * Привязать вызывающий поток к процессорам.
 * Маска не наследуется.
 * 
 * @param mask  Маска процессоров (0 - ничего не делать)
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
bool SetCpuMask(uint32_t mask);

/*!
 * Привязать вызывающий поток к заданному процессору.
 * Маска не наследуется.
 * 
 * @param i  Индекс процессора
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
static inline bool SetCpuIndex(int i) {
	return SetCpuMask(((uint32_t)1) << i);
}

/*!
 * Привязать вызывающий поток к процессорам,
 * и унаследовать маску.
 * 
 * @param mask  Маска процессоров (0 - ничего не делать)
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
bool SetCpuMaskInherit(uint32_t mask);

/*!
 * Привязать вызывающий поток к заданному процессору,
 * и унаследовать маску.
 * 
 * @param i  Индекс процессора
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
static inline bool SetCpuIndexInherit(int i) {
	return SetCpuMaskInherit(((uint32_t)1) << i);
}

#endif

}
#endif
