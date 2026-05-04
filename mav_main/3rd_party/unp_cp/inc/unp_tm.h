/*
 * unp_tm.h
 * 
 * Телеметрия
 * 
 *  Created on: 26 апр. 2024 г.
 *      Author: kudr
 */

#ifndef INC_UNP_TM_H_
#define INC_UNP_TM_H_

#include <inttypes.h>

#include <string>

namespace UNP {

class Tm {
	std::string dir, prefix, suffix;
	int digits;
	
	unsigned recId; //!< Идентификатор последней записи
	
public:
	/*!
	 * Конструктор.
	 * Определит recId по списку файлов в директории. Имя файла: <префикс (всё, кроме цифр)><id записи>_<дата><суффикс> .
	 * Если директория не существует, то будет создана, но не рекурсивно.
	 * 
	 * @param dir    Директория (пустая строка будет заменена на точку)
	 * @param digits Количество цифр идентификатора
	 */
	Tm(const std::string &dir, int digits, const std::string &prefix, const std::string &suffix);
	virtual ~Tm() {}
	
	/*!
	 * Получить имя файла для следующей записи
	 */
	std::string Next();
	
	/*!
	 * Освободить место
	 * 
	 * @param avlSize Желаемое свободное место (в байтах)
	 */
	void FreeSpace(uint64_t avlSize);
};

}
#endif /* INC_UNP_TM_H_ */
