/*
 * unp_pci.h
 *
 * Работа с PCI
 *
 *  Created on: 16.03.2016
 *      Author: kudr
 */

#ifndef UNP_PCI_H_
#define UNP_PCI_H_

#ifdef __QNXNTO__

#include <inttypes.h>
#include <hw/pci.h>

namespace UNP {

//! Устройство на шине PCI
//! (процесс должен быть запущен от root и предварительно вызвать ThreadCtl(_NTO_TCTL_IO, 0))
class PciDev {
	void *devHandle;      //!< Хандлер устройства (возвращает pci_attach_device())
	pci_dev_info devInfo; //!< Информация по PCI-устройству
	
public:
	PciDev();
	virtual ~PciDev();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * @param vendorId               ID вендора
	 * @param deviceId               ID устройства
	 * @param deviceIndex            Индекс устройства
	 * @param devicePciMasterEnable  Разрешение устройству быть мастером PCI
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(unsigned vendorId, unsigned deviceId, unsigned deviceIndex = 0, bool devicePciMasterEnable = false);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
	inline const pci_dev_info& GetDevInfo() const { return devInfo; }
	
private:
	PciDev(const PciDev&);
	PciDev& operator=(const PciDev&);
};

//! BAR устройства на шине PCI
class PciBar {
public:
	volatile uint8_t  *ptr;     //!< Указатель на отображаемую область из соответствующего BAR
	size_t            size;     //!< Размер отображаемой области
	off64_t           physAddr; //!< Физический адрес
	
public:
	PciBar();
	virtual ~PciBar();
	
	/*!
	 * Инициализация
	 * (может быть повторной)
	 * @param dev        Устройство на шине PCI
	 * @param barNumber  Номер bar
	 * @return "Истина" - успех
     *         "Ложь"   - ошибка
	 */
	bool Init(const PciDev &dev, unsigned barNumber);
	
	/*!
	 * Освобождение ресурсов
	 * (можно не вызывать, т.к. автоматически вызовется в деструкторе)
	 */
	void Release();
	
private:
	PciBar(const PciBar&);
	PciBar& operator=(const PciBar&);
};

}
#endif
#endif
