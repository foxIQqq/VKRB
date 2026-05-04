/*
 * unp_fs.h
 *
 * Файловая система
 *
 *  Created on: 21.08.2015
 *      Author: kudr
 */

#ifndef UNP_FS_H_
#define UNP_FS_H_

namespace UNP {

/*!
 * Проверка на директорию
 * (с разрешением симв. ссылок)
 * 
 * @param path Путь
 * 
 * @return "Истина" - да
 *         "Ложь" - нет или ошибка
 */
bool IsDir(const char *path);

/*!
 * Проверка на регулярный файл
 * (с разрешением симв. ссылок)
 * 
 * @param path Путь
 * 
 * @return "Истина" - да
 *         "Ложь" - нет или ошибка
 */
bool IsReg(const char *path);

/*!
 * Проверка на то, что файл не существует
 * 
 * @param path Путь к файлу
 * 
 * @return "Истина" - файл не существует;
 *         "Ложь" - файл существует или ошибка
 */
bool FileNotExist(const char *path);

/*!
 * Создание поддиректории (родительские директории не создаются)
 * 
 * @param path Путь к директории
 * 
 * @return "Истина" - успех
 *         "Ложь" - ошибка
 */
bool MkDir(const char *path);

/*!
 * Создание директории со всеми необходимыми родительскими директориями
 * 
 * @param path Путь к директории
 * 
 * @return "Истина" - успех
 *         "Ложь" - ошибка
 */
bool MkDirA(const char *path);

/*!
 * Рекурсивное удаление файла
 * 
 * @param path Путь к файлу
 * 
 * @return "Истина" - успех
 *         "Ложь" - ошибка
 *         (Отстуствие файла за ошибку не считается)
 */
bool RemoveA(const char *path);

/*!
 * Копирование файла
 * 
 * @param src
 * @param dst
 * @param pwork
 * 
 * @return "Истина" - успех
 *         "Ложь" - ошибка
 */
bool CopyFile(const char *src, const char *dst, const volatile bool *pwork = NULL);

}
#endif
