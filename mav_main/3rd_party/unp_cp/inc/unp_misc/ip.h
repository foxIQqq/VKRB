/*
 * ip.h
 *
 *  Created on: 9 мар. 2023 г.
 *      Author: kudr
 */

#ifndef INC_UNP_MISC_IP_H_
#define INC_UNP_MISC_IP_H_

#include <stdlib.h>
#include <inttypes.h>

namespace UNP {

uint16_t IpCalcChecksum(const void *buf, size_t size);

}
#endif /* INC_UNP_MISC_IP_H_ */
