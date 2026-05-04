/*
 * unp_misc/daemonize.h
 *
 * Вспомогательные низкоуровневые функции для работы с фоновыми процессами (daemons).
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__DAEMONIZE_H_
#define UNP_MISC__DAEMONIZE_H_

#include <inttypes.h>
#include <unistd.h>

#define UNP_DAEMONIZE_NOCHDIR   0x01 //!< Не менять текущую директорию на "/"

namespace UNP {

/*!
 * Перевод процесса в фоновый режим.
 * (Все открытые файловые дескрипторы будут закрыты;
 * stdin, stdout, stderr будут направлены на /dev/null.)
 * В случае ошибки процесс будет завершён.
 * 
 * @param flags Флажки (см. UNP_DAEMONIZE_*)
 */
void daemonize(uint32_t flags = 0);

/*!
 * Проверяет, работает ли процесс, по наличию блокировки файла.
 * В файл запишет pid (опционально).
 * 
 * @param lockFile       Файл
 * @param extraOpenFlags Дополнительные флаги для open() (например, O_CLOEXEC)
 * @param writePid2File  Записывать или нет в файл pid процесса
 * @param ppid           Если не NULL, то туда запишет pid работающего процесса
 * @param pfd            Если не NULL, то туда запишет файловый дескриптор
 * 
 * @return  0 - не запущена;
 *         >0 - запущена;
 *         <0 - ошибка (установит errno)
 */
int already_running_by_lock(const char *lockFile, int extraOpenFlags, bool writePid2File, pid_t *ppid, int *pfd);

/*!
 * Проверяет, работает ли процесс, по pid из файла.
 * В файл запишет pid.
 * 
 * @param pidFile Файл
 * @param ppid    Если не NULL, то туда запишет pid работающего процесса
 * 
 * @return  0 - не запущена;
 *         >0 - запущена;
 *         <0 - ошибка (установит errno)
 */
int already_running_by_proc_exist(const char *pidFile, pid_t *ppid);

/*!
 * Прочитать pid из файла
 * 
 * @param fd Дескриптор
 * 
 * @return Прочитанный pid или
 *         <0 в случае ошибки (установит errno) 
 */
pid_t read_pid(int fd);

/*!
 * Записать pid в файл
 * 
 * @param fd  Дескриптор
 * @param pid pid (если <= 0, то без записи значения - только truncate())
 * 
 * @return  0 - успех;
 *         <0 - ошибка (установит errno)
 */
int write_pid(int fd, pid_t pid);

}
#endif
