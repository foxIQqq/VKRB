/*
 * unp_misc/io.h
 *
  Вспомогательные низкоуровневые функции для работы с вводом/выводом.
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__IO_H_
#define UNP_MISC__IO_H_

#include <sys/types.h>
#include <stdio.h>

namespace UNP {

/*!
 * Чтение данных из файла.
 * Функция в цикле пытается считать указанное количество байт.
 * 
 * @param fd     Дескриптор
 * @param buf    Буфер
 * @param nBytes Количество байт
 * 
 * @return Количество прочитанных байт или
 *         <0 в случае ошибки (установит errno)
 */
ssize_t readn(int fd, void *buf, size_t nBytes);

/*!
 * Запись данных в файл.
 * Функция в цикле пытается записать указанное количество байт.
 * 
 * @param fd     Дескриптор
 * @param buf    Буфер
 * @param nBytes Количество байт
 * 
 * @return Количество записанных байт или
 *         <0 в случае ошибки (установит errno)
 */
ssize_t writen(int fd, const void *buf, size_t nBytes);

/*!
 * Обычный getline() (штатный getline() есть не на всех платформах).
 * Читает строки. Переводы строк не удаляет.
 * @param line
 * @param size
 * @param stream
 * @return
 */
ssize_t getline(char **line, size_t *size, FILE *stream);

}
#endif
