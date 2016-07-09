#include "pci_dev.h"
#include "PCI_CODES.h"
#include "printf.h"
#include "io.h"

u32 bus = 0;  		//256
u32 device = 0;		//32
u32 function = 0;	//8

void ShowPCIDevices(enum PCI_Dev dev)
{
	while(showDeviceInfo())
	{
		if(dev == NEXT)
		{
			nextDev();
		}
		else if(dev == PREV)
		{
			prevDev();
		}

	}
	if(dev == NEXT)
	{
		nextDev();
	}
	else if(dev == PREV)
	{
		prevDev();
	}

}
void nextDev()
{
	if(function == 7)
	{
		function = 0;
		if(device == 31)
		{
			device = 0;

			if(bus == 255)
			{
				bus = 0;
			}
			else 
			{
				bus++;
			}
		}
		else
		{
			device++;
		}
	}
	else
	{
		function++;
	}
}
void prevDev()
{
	if(function == 0)
	{
		function = 7;
		if(device == 0)
		{
			device = 31;
			if(bus == 0)
				bus = 255;
			else 
				bus--;
		}
		else
			device--;
	}
	else
		function--;
}

u32 showDeviceInfo()
{
	u32 configAddress = calculateAddress(bus, device, function, 0x00); 
	u32 RegData = getRegData(configAddress);   

	if (RegData == -1)
	{
		return 1;
	}
	

	u32 DeviceID = RegData >> 16;               
	u32 VendorID = RegData - (DeviceID << 16);

	
	configAddress = calculateAddress(bus, device, function, 0x08);
	RegData = getRegData(configAddress);

	u32 ClassID = RegData >> 8;                             
	u32 RevisionID = RegData - (ClassID << 8);

	u32 BaseClassCode = ClassID >> 16;              
	u32 Progu32erface = ClassID - (BaseClassCode << 16); 
	u32 SubClassCode = Progu32erface >> 8;             
	Progu32erface = Progu32erface - (SubClassCode << 8);    

	
	configAddress = calculateAddress(bus, device, function, 0x2C);
	RegData = getRegData(configAddress);

	u32 SubsysID = RegData >> 16;
	u32 SubsysVendID = RegData - (SubsysID << 16);

	printf("Bus: %x, device: %x, function: %x\n", bus, device, function); 
	printf("Device ID: %x, Vendor ID: %x, Class ID: %x-%x-%x\n", DeviceID, VendorID, BaseClassCode, SubClassCode, Progu32erface);
	printf("Revision ID: %x, Subsystem ID: %x, Subsystem Vendor ID: %x\n", RevisionID, SubsysID, SubsysVendID);
	decodeNames(DeviceID, VendorID, BaseClassCode, SubClassCode, Progu32erface);
	printf("-------------------------------------------------------------------------------\n");

	return 0;
}


u32 getRegData(u32 configAddress)
{
	u32 regData;

	/*__asm__("movl %1, %%eax;"
		"movw $0x0CF8, %%dx;"
        	"outl %%eax, %%dx;" 
		"movw $0x0CFC, %%dx;"   
        	"inl %%dx, %%eax;"    
		"movl %%eax, %0;"
		 :"=r"(regData)
		 :"r"(configAddress)
		);*/

	/*outportl(configAddress, 0x0CF8);
	inportl(0x0CFC, regData);*/
	out32(0x0CF8, configAddress);
	in32(0x0CFC, &regData);

	return regData;
}


u32 calculateAddress(u32 bus, u32 device, u32 function, u32 reg)
{
	u32 address = 1;
	address = address << 15;
	address += bus;      
	address = address << 5;      
	address += device;      
	address = address << 3;
	address += function;     
	address = address << 8;
	address += reg;
	return address;
}

void decodeNames(u32 DeviceId, u32 VendorId, u32 BaseClass, 
					 u32 SubClass, u32 ProgIf)
{
	u32 i;
	for(i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
	{
		if(PciClassCodeTable[i].BaseClass == BaseClass && PciClassCodeTable[i].SubClass == SubClass 
			&& PciClassCodeTable[i].ProgIf == ProgIf)		
			printf("%s ( %s %s)\n", PciClassCodeTable[i].BaseDesc, PciClassCodeTable[i].SubDesc, PciClassCodeTable[i].ProgDesc);		
	}

	for(i = 0; i < PCI_DEVTABLE_LEN; i++)
	{
		if(PciDevTable[i].VenId == VendorId && PciDevTable[i].DevId == DeviceId)		
			printf("%s, %s\n", PciDevTable[i].Chip, PciDevTable[i].ChipDesc);		
	}

	for(i = 0; i < PCI_VENTABLE_LEN; i++)
	{
		if(PciVenTable[i].VenId == VendorId)		
			printf("%s\n", PciVenTable[i].VenFull);		
	}
}