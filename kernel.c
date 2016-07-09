#include "printf.h"
#include "screen.h"
#include "desc.h"
#include "callrealmode.h"
#include "pci_dev.h"
#include "discs.h"


struct segdesc g_GDT[5];

void SetupDescTables(struct segdesc *GDT_base);
u8 GetScanCode();
void DrawFractal(void);
void ShowPCIDevices(enum PCI_Dev dev);
void HandleInput();

void kmain(void)
{
	SetupDescTables(g_GDT);

	clear();
	printf("--Kernel started!--\n");

	while(1)
		HandleInput();

    /*u64 ram_size = GetAvalibleRAMSize ();
    printf("ram_size = %llu(%lluMb)\n", ram_size, ram_size / 0x100000);*/	
}

void HandleInput()
{
	switch(GetScanCode())
	{
		case 0x50 :
		{
			//int i;
			//MoveCursor(DOWN);

			clear_screen();
			nextDisc();

			/*for(i = 0; i < 3; i++)
				ShowPCIDevices(NEXT);*/

			break;
		}
		case 0x48 :
		{
			int i;
			//MoveCursor(UP);

			clear_screen();
			for(i = 0; i < 3; i++)
				ShowPCIDevices(NEXT);

			break;
		}
		case 0x49 :
		{
			clear_screen();

			DrawFractal();
			break;
		}
	}
}




