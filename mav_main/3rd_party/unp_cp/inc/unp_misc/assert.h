/*
 * unp_misc/assert.h
 *
 * Вспомогательные низкоуровневые функции для работы с утверждениями.
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__ASSERT_H_
#define UNP_MISC__ASSERT_H_

// private
void _unp_assert_fail(const char *assertion, const char *file, int line, const char *func) __attribute__((__noreturn__));
static inline void _unp_noop(void) {}

//! Если выражение ложно, печать диагностического сообщения на stderr и завершение процесса.
//! В отличии от стандартной реализации работает без проверки макроса NDEBUG.
#define unp_assert(cond) ((!(cond)) ? _unp_assert_fail(#cond,__FILE__,__LINE__,__FUNCTION__) : _unp_noop())

#endif
