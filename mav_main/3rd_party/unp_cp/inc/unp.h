/*
 * unp.h
 * 
 * Основной заголовочный файл библиотеки unp (Unix Network Programming)
 * (на основе материалов книг У.Р. Стивенса).
 * 
 *  Created on: 19.08.2015
 *      Author: kudr
 */

#ifndef UNP_H_
#define UNP_H_

#include "unp_log.h"

namespace UNP {

/*!
 * Инициализация библиотеки.
 * Вызывать не обязательно.
 * До вызова этой функции логирование осуществляется на std и в исключительных ситуациях выдаются исключения C++ (см. assertThrow).
 * 
 * @param logger      Логирование.
 *                    Класс должен обеспечивать безопасную работу с одним экземпляров из нескольких потоков.
 * 
 * @param assertThrow Выдача исключения C++ (throw) при возникновении исключительной ситуации (assertion failed):
 *                      true  - да (std::runtime_error)
 *                      false - нет (завершение процесса)
 * 
 * @return "Истина" - успех;
 *         "Ложь"   - ошибка
 */
bool LibraryInit(Logger &logger, bool assertThrow = false);

/*!
 * Альтернатива LibraryInit(Logger&).
 * 
 * Отличается отсутствием необходимости заранее создавать логер -
 * здесь будет создан логер класса LoggerSys,
 * повторный вызов данной функции логер не перенастроит.
 * 
 * @param appName     Имя приложения
 * @param toStd       Включить логирование на std
 * @param toJournal   Включить логирование в системный журнал
 * @param assertThrow Выдача исключения C++ (см. описание LibraryInit(Logger&, ...))
 * 
 * @return Указатель на логер при успехе (не удалять), NULL - при ошибке
 */
Logger* LibraryInit(const char *appName, bool toStd = true, bool toJournal = true, bool assertThrow = false);

//! Завершение работы с библиотекой (вызывать не обязательно)
void LibraryFin();

/*!
 * Получение указателя на логер
 * @return Указатель (гарантированно не NULL, не удалять)
 */
Logger* GetLogger();

}

#endif
