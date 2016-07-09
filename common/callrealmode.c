#include "printf.h"
#include "types.h"
#include "string.h"
#include "segment.h"
#include "callrealmode_asm.h"
#include "callrealmode.h"

// interrupts and paging must be disabled 

u8 current_row = 0;
u8 current_column = 0;
u8 current_page = 0;

static void callrealmode_Call(struct callrealmode_Data *p_param)
{
    u16 sp16;
    u32 sp32;
    
    // copy 16 bit code and stack
    // первый вызов memcpy копирует 16ти битный код между 
    // метками callrealmode_start и callrealmode_end
    // по адресу CALLREALMODE_OFFSET < 1Mb. Это делается 
    // из за того что, код в RM не может обращаться к 
    // памяти выше 1Mb.
    memcpy ((u8*)CALLREALMODE_OFFSET, &callrealmode_start, 
            &callrealmode_end - &callrealmode_start);
    // перед кодом располагаем стек, который не будет затирать код, так как растет вниз.
    // параметры для вызова в RM располагаются на стеке, и новое значение регистра SP 
    // вычисляется с учетом этого
    sp16 = CALLREALMODE_OFFSET - sizeof(*p_param);
    // второй вызов memcpy копирует параметры на стек
    memcpy ((void*)(u32)sp16, p_param, sizeof(*p_param));
    
    __asm__ volatile (
        "mov %%esp,%0\n"    // Сохраняем ресистр ESP в переменную sp32
        "mov %1,%%ds \n"    // Загружаем 16ти битный селектор данных в 
        "mov %1,%%es \n"    // регистры DS, ES, FS, GS, SS
        "mov %1,%%fs \n"    //
        "mov %1,%%gs \n"    //
        "mov %1,%%ss \n"    //
        "mov %2,%%esp\n"    // Переходим на 16ти битный стек по адресу sp16
        "pusha       \n"    // Сохраняем регистры общего назначения
        "lcall %3,%4 \n"    // Загружаем 16ти битный селектор кода в CS 
                            // и переходим на адрес CALLREALMODE_OFFSET.
                            // На стеке сохраняется текущий CS и EIP,
                            // которые будут использованны инструкцией 
                            // lretl для возврата в 32х битный код
        "popa        \n"    // востанавливаем сохраненные регистры общего назначения
        "mov %5,%%ds \n"    // Загружаем 32х битный селектор данных в 
        "mov %5,%%es \n"    // регистры DS, ES, FS, GS, SS
        "mov %5,%%fs \n"    //
        "mov %5,%%gs \n"    //
        "mov %5,%%ss \n"    //
        "mov %0,%%esp\n"    // Переходим на 32х битный стек, адрес
                            // которого сохранен в переменной sp32
        : "=&a" (sp32)              // %0 – Input
        : "b" ((u32)SEG_SEL_DATA16) // %1 - Output
        , "c" ((u32)sp16)           // %2 - Output
        , "i" ((u32)SEG_SEL_CODE16) // %3 - Output
        , "i" (CALLREALMODE_OFFSET) // %4 - Output
        , "d" ((u32)SEG_SEL_DATA32) // %5 - Output
        );

    // копируем с результаты работы с 16ти битного стека в p_param
    memcpy (p_param, (void*)(u32)sp16, sizeof(*p_param));
}

u64 GetAvalibleRAMSize()
{
    struct callrealmode_Data param; // структура, в которой передаются параметры 
                                    // для кода в RM, и возвращается результат
    u64 avalible_ram_sz = 0;

    param.func = CALLREALMODE_FUNC_GETSYSMEMMAP;
    param.getsysmemmap.next_num = 0;
    do
    {
        param.getsysmemmap.num = param.getsysmemmap.next_num;
        callrealmode_Call(&param);  // int 0x15, где EBX = param.getsysmemmap.num
                                    //   EAX = 0xE820, EDX = 0x534d4150, ECX = 20
                                    //   ES:DI = ¶m.getsysmemmap.base
                                    // после вызова EBX сохраняется
                                    //   param.getsysmemmap.next_num = EBX

        // нас интересуют только доступные диапазоны с типом SYSMEMMAP_TYPE_AVAILABLE
        if (SYSMEMMAP_TYPE_AVAILABLE == param.getsysmemmap.type)
        {
            avalible_ram_sz += param.getsysmemmap.len;
        }
        
        printf("n 0x%08X nn 0x%08X b 0x%08llX l 0x%08llX(%lldMb) t 0x%08X\n",
            param.getsysmemmap.num, 
            param.getsysmemmap.next_num,
            param.getsysmemmap.base, 
            param.getsysmemmap.len, 
            param.getsysmemmap.len / 0x100000, 
            param.getsysmemmap.type);
    }
    while (param.getsysmemmap.next_num);

    return avalible_ram_sz;
}

u8 GetScanCode()
{
    struct callrealmode_Data param;

    param.func = CALLREALMODE_FUNC_GETSCANCODE; 

    callrealmode_Call(&param);

    return param.getscancode.scan_code;
    
}

void MoveCursor(enum MoveCursor_Func func)
{
    struct  callrealmode_Data param;

    param.func = CALLREALMODE_FUNC_MOVECURSOR;

    switch(func)
    {
        case BEGIN : 
        {
            current_page = 0;
            current_column = 0;
            current_row = 0;

            break;
        }
        case UP :
        {
            current_row--;

            break;
        }
        case DOWN :
        {
            current_row++;
            
            break;
        }
    }

    param.movecursor.column = current_column;
    param.movecursor.row = current_row;
    param.movecursor.page = current_page;


    callrealmode_Call(&param);

    return;
}