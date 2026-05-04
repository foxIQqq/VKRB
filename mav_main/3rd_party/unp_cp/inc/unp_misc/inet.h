/*
 * unp_misc/inet.h
 *
 * Вспомогательные низкоуровневые функции для работы с сетью.
 * 
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_MISC__INET_H_
#define UNP_MISC__INET_H_

#include <netinet/in.h>

#include <list>
#include <string>

namespace UNP {

class IfiInfo {
public:
	unsigned long flags; //!< Флажки (IFF_BROADCAST и т.д.)
	in_addr_t addr;      //!< Первичный адрес
	in_addr_t netMask;   //!< Маска сети
	in_addr_t brdAddr;   //!< Широковещательный адрес
	
	IfiInfo();
};

/*!
 * Получение информации о сетевых интерфейсах
 * 
 * @param dst Результат
 * 
 * @return  0 - успех;
 *         <0 - ошибка (установит errno)
 */
int get_ifi_info(std::list<IfiInfo> &dst);

/*!
 * Конвертация сетевого адреса в строку
 * 
 * @param addr
 * @return
 */
std::string inet_addr_ntop(in_addr_t addr);

}
#endif
