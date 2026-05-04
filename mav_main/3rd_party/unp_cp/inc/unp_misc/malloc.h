/*
 * unp_misc/malloc.h
 *
 * Вспомогательные низкоуровневые функции для работы с памятью.
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__MALLOC_H_
#define UNP_MISC__MALLOC_H_

#include <stdlib.h>

namespace UNP {

/*!
 * Модифицированная версия функции realloc().
 * Отличия:
 *  1. Освобождает память в случае ошибки.
 *  2. Если size <= 0, то возвращает NULL (стандартный realloc() мог вернуть не NULL).
 * 
 * @param ptr
 * @param size
 * @return
 */
void* realloc(void *ptr, size_t size);

}
#endif
