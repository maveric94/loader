#ifndef PCI_DEV_H
#define PCI_DEV_H

#include "types.h"

enum PCI_Dev
{
	NEXT = 0,
	PREV = 1,
};

void ShowPCIDevices(enum PCI_Dev dev);
u32 showDeviceInfo();
void decodeNames(u32, u32, u32, u32 , u32);
u32 calculateAddress(u32, u32, u32, u32);
u32 getRegData(u32);


#endif