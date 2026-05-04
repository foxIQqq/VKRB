/*
 * unp_pci.cc
 *
 *  Created on: 16.03.2016
 *      Author: kudr
 */

#ifdef __QNXNTO__

#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "unp_private.h"

#include "unp_pci.h"

#define MAP_FAILED_V  static_cast<volatile uint8_t*>(MAP_FAILED)

namespace UNP {

#define ASSERT_RET(v, prefix) \
	if(!(v)) { \
		l.AddSysError(errno, PREFIX prefix); \
		return false; \
	}

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX  "PciDev: "

static int pciHandle = -1; //!< Хандлер шины PCI
static pthread_once_t pciOnceControl = PTHREAD_ONCE_INIT;

static void PciInit() {
	pciHandle = pci_attach(0);
	if(pciHandle == -1)
		l.AddSysError(errno, PREFIX "pci_attach");
}

PciDev::PciDev() : devHandle(NULL) {
	bzero(&devInfo, sizeof(devInfo));
}

PciDev::~PciDev() {
	Release();
}

bool PciDev::Init(unsigned vendorId, unsigned deviceId, unsigned deviceIndex, bool devicePciMasterEnable) {
	Release();
	
	pthread_once(&pciOnceControl, PciInit);
	if(pciHandle == -1) {
		l.AddError(PREFIX "PCI not attached");
		return false;
	}
	
	{
		int idx = deviceIndex;
		int flags = 0
				| PCI_SHARE
				| PCI_SEARCH_VENDEV
				| PCI_INIT_ALL
			;
		
		if(devicePciMasterEnable)
			flags |= PCI_MASTER_ENABLE;
		
		devInfo.VendorId = vendorId;
		devInfo.DeviceId = deviceId;
		
		devHandle = pci_attach_device(devHandle, flags, idx, &devInfo);
		if(devHandle == NULL) {
			l.AddError("Can't attach to PCI device (vid=0x%X did=0x%X idx=%u masterEnable=%d): %s", vendorId, deviceId, deviceIndex, (int)devicePciMasterEnable, strerror(errno));
			return false;
		}
	}
	
	return true;
}

void PciDev::Release() {
	if(devHandle) {
		if(pci_detach_device(devHandle) != PCI_SUCCESS)
			l.AddError(PREFIX "%s error", "pci_detach_device");
		devHandle = NULL;
	}
}

#ifdef PREFIX
#undef PREFIX
#endif
#define PREFIX  "PciBar: "

PciBar::PciBar() : ptr(MAP_FAILED_V), size(0), physAddr(0) {
}

PciBar::~PciBar() {
	Release();
}

bool PciBar::Init(const PciDev &dev, unsigned barNumber) {
	Release();
	
	if(barNumber >= 6) {
		l.AddError(PREFIX "barNumber=%u too big", barNumber);
		return false;
	}
	
	const pci_dev_info &devInfo = dev.GetDevInfo();
	
	size = devInfo.BaseAddressSize[barNumber];
	physAddr = PCI_MEM_ADDR(devInfo.CpuBaseAddress[barNumber]);
	ptr = static_cast<volatile uint8_t*>(mmap_device_memory(
				NULL,
				size,
				PROT_READ | PROT_WRITE | PROT_NOCACHE,
				0,
				physAddr
			));
	ASSERT_RET(ptr != MAP_FAILED_V, "mmap_device_memory")
	
	return true;
}

void PciBar::Release() {
	if(ptr != MAP_FAILED_V) {
		if(munmap_device_memory(const_cast<uint8_t*>(ptr), size) == -1)
			l.AddSysError(errno, "munmap_device_memory");
		ptr = MAP_FAILED_V;
	}
}

}
#endif
