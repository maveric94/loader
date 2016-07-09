#include "discs.h"
#include "printf.h"
#include "io.h"

u16 alt_state_reg[2] = {0x3F6, 0x376};
u16 state_cmd_reg[2] = {0x1F7, 0x177};

u16 data[256];

u32 ch = 0;
u32 dev = 0;

void nextDisc()
{
	printf("Channel %d Device %d", ch, dev);

	if(getDeviceInfo(dev, ch))
		printInfo();
	else
		printf("\nNo device was found.");


	if(dev == 1)
	{
		dev = 0;

		if(ch == 1)
			ch  = 0;
		else 
			ch++;
	}
	else
		dev++;
}

u32 waitReady(u32 channel)
{
	u32 i;

	for (i = 0; i < 100000; i++)
	{
		u8 state;

		in8(alt_state_reg[channel], &state);   

		if(state & (1 << 6)) 
			return 1; 
	}
	return 0;
}


void  WaitDeviceBusy(u32 channel)
{
	u8 state;

	do 
	{
		in8(alt_state_reg[channel], &state); 
	}
	while (state & (1 << 7));
}

u32  getDeviceInfo(u32 device, u32 channel)
{	
	u8 cmds[2] = {0xA1, 0xEC};

	u16 DH_reg[2] = {0x1F6, 0x176};

	u16 data_reg[2] = {0x1F0, 0x170};

	u32 i;

	for (i = 0; i < 2; i++)
	{
		u8 curState;
		u8 regData = (device << 4) + (7 << 5); 

		WaitDeviceBusy(channel);

		out8(DH_reg[channel], regData); 

		if(!waitReady(channel))	
			return 0;		

		out8(state_cmd_reg[channel], cmds[i]);

		WaitDeviceBusy(channel);

		in8(state_cmd_reg[channel], &curState);

		if(!(curState & (1 << 3)))  
		{
			if(i == 1) 
				return 0;

			continue;
		}
		else 
			break;
	}

	for(i = 0; i < 256; i++ )
		in16(data_reg[channel], &data[i]);
	
	return 1;
}

void printInfo()
{
	u32 i;
	printf("\nDevice model: ");
	for(i = 27; i <= 46; i++)
		printf("%c%c", data[i] >> 8, data[i] & 0x00FF );

	printf("\nSerial number: ");
	for(i = 10; i <= 19; i++ )
		printf("%c%c", data[i] >> 8, data[i] & 0x00FF );

	printf("\nFirmware version: ");
	for(i = 23; i <= 26; i++ )
		printf("%c%c", data[i] >> 8, data[i] & 0x00FF );

	printf("\nData exchange type: ");
	
	if(data[0] & (1 << 15))	
		printf("ATAPI\n");
	else 
		printf("ATA\n");

	u32 secs = data[60];
	secs += (data[61] << 16);

	
	if(!(data[0] & (1 << 15)))
		printf("Size: %lld.\n", secs * 512);

	if(data[81] != 0x0000 && data[81] != 0xFFFF)
		printf("Specification version: %d\n", data[81]);
		
	printf("\nPIO: ");
	for(i = 0; i < 2; i++)
		printf("%d - %s ", (i + 3), (data[64] & (1 << i)) ? "yes" : "no");

	printf("\nMWDDMA: ");
	for(i = 0; i < 3; i++)
		printf("%d - %s ", i, (data[63] & (1 << i)) ? "yes" : "no");

	printf("\nUDMA: ");
	for(i = 0; i < 6; i++)
		printf("%d - %s ", i, (data[88] & (1 << i)) ? "yes" : "no");

	printf("\nATA: ");
	for(i = 1; i < 8; i++)
		printf("%d - %s ", i, (data[80] & (1 << i)) ? "yes" : "no");
}