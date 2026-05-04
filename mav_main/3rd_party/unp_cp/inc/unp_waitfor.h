/*
 * unp_waitfor.h
 * 
 * Ожидание появления файла.
 * 
 * При ошибке бросается исключение типа int с кодом ошибки из errno.
 * 
 *  Created on: 20 нояб. 2023 г.
 *      Author: kudr
 */

#ifndef INC_UNP_WAITFOR_H_
#define INC_UNP_WAITFOR_H_

#ifdef __QNXNTO__

#include <sys/siginfo.h>

namespace UNP {

class Waitfor {
	int chId{-1};
	int coId{-1};
	int notifyId{-1};
	sigevent event; //-V730_NOINIT
	
public:
	Waitfor();
	virtual ~Waitfor();
	
	/*!
	 * Самый простой вариант ожидания, работающий через process manager.
	 * (для работы с вложенными директориями и файлами нужен ещё периодический опрос).
	 * 
	 * @param path    Путь, который ожидаем
	 * @param appear  Если true, то ждём появления пути, иначе - исчезновения
	 * 
	 * Точка отмены: да.
	 */
	void WaitProcmgr(const char *path, bool appear = true);
	
	/*!
	 * Ожидание любого изменения в ФС через process manager
	 */
	void WaitProcmgr();
	
private:
	void Release();
};

}
#endif
#endif /* INC_UNP_WAITFOR_H_ */
