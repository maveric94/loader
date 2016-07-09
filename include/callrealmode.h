#ifndef _CALLREALMODE_H
#define _CALLREALMODE_H

#include "types.h"

enum MoveCursor_Func
{
	UP = 0,
	DOWN = 1,
	BEGIN = 2
};

u64 GetAvalibleRAMSize();
u8 GetScanCode();
void MoveCursor(enum MoveCursor_Func func);

#endif