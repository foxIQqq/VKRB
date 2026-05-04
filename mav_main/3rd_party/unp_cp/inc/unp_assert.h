/*
 * unp_assert.h
 *
 * Утверждения.
 * 
 * При возникновении исключительной ситуации (assertion failed)
 * в лог выводится соответствующая информация,
 * и процесс завершается / выдаётся исключение (см. UNP::LibraryInit()). 
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_ASSERT_H_
#define UNP_ASSERT_H_

// private
void _UNP_AssertFail(const char *assertion, const char *file, int line, const char *func) __attribute__((__noreturn__));
static inline void _UNP_Noop(void) {}

/*!
 * Проверка выражения
 * 
 * @param cond Выражение
 */
#define UNP_Assert(cond) ((!(cond)) ? _UNP_AssertFail(#cond,__FILE__,__LINE__,__FUNCTION__) : _UNP_Noop())

namespace UNP {

/*!
 * Проверка выражения с доп. логированием системной ошибки
 * 
 * @param cond
 * @param err
 * @param prefix
 */
void AssertSysError(bool cond, int err, const char *prefix);

/*!
 * Завершение работы с сообщением
 * @param msg
 */
void AssertFail(const char *format, ...) __attribute__((__noreturn__)) __attribute__((format (__printf__, 1, 2)));

}

#endif /* UNP_ASSERT_H_ */
