#ifndef DISCS_H
#define DISCS_H

#include "types.h"

void WaitDeviceBusy(u32 channelNum);
u32 getDeviceInfo(u32 devNum, u32 channelNum);
void printInfo();
u32 waitReady(u32 channelNum);

void nextDisc();


#endif