/*
 * string.h
 * 
 * Вспомогательные низкоуровневые функции для работы со строками.
 * 
 *  Created on: 22.03.2019
 *      Author: kudr
 */

#ifndef UNP_MISC__STRING_H_
#define UNP_MISC__STRING_H_

#include <inttypes.h>
#include <string>

namespace UNP {

/*!
 * Конвертация строки с номерами бит (нумерация с 0)
 * в 32-битную маску.
 * 
 * Пример:
 *  uint32_t mask = UNP::string_bits_to_mask32("0,4,8"); // 0x111
 * 
 * @param s   Строка
 * @param sep Разделитель
 * 
 * @return Маска
 */
uint32_t string_bits_to_mask32(const char *s, char sep = ',');

/*!
 * Конвертация 32-битной маски
 * в строку с номерами бит.
 * 
 * Пример - см. string_bits_to_mask32().
 * 
 * @param mask Маска
 * @param sep  Разделитель
 * 
 * @return Строка
 */
std::string string_bits_from_mask32(uint32_t mask, char sep = ',');

}

#endif
