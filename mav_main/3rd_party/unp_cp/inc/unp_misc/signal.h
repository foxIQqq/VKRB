/*
 * unp_misc/signal.h
 *
 * Вспомогательные низкоуровневые функции для работы с сигналами.
 * 
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__SIGNAL_H_
#define UNP_MISC__SIGNAL_H_

#include <signal.h>

namespace UNP {

typedef void (*SigFunc)(int);

/*!
 * Надёжная (reliable) версия функции signal() с использованием sigaction() стандарта POSIX.
 * Для SIGALRM установит флаг SA_INTERRUPT,
 * для остальных - SA_RESTART.
 * 
 * @param signo Номер сигнала
 * @param func  Обработчик
 * 
 * @return Предыдущее значение обработчика или SIG_ERR в случае ошибки (errno будет установлен в EINVAL).
 */
SigFunc signal_r(int signo, SigFunc func);

}
#endif
