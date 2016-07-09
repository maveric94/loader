#include "desc.h"
#include "string.h"

static void SetSegDesc(struct segdesc *d, u32 limit, u32 base, enum segdesc_type type,
                       enum segdesc_s s, unsigned int dpl, unsigned int p,
                       unsigned int avl, enum segdesc_l l, enum segdesc_d_b d_b)
{
    d->base_15_0 = base;
    d->base_23_16 = base >> 16;
    d->type = type;
    d->s = s;
    d->dpl = dpl;
    d->p = p;
    d->avl = avl;
    d->l = l;
    d->d_b = d_b;
    d->base_31_24 = base >> 24;
    
    if (limit <= 0xFFFFF) 
    {
        d->g = 0;
        d->limit_15_0 = limit >> 0;
        d->limit_19_16 = limit >> 16;
    } 
    else 
    {
        d->g = 1;
        d->limit_15_0 = limit >> 12;
        d->limit_19_16 = limit >> 28;
    }
} 

void SetupDescTables(struct segdesc *GDT_base)
{
    // SEG_SEL_NULL
    memset(&GDT_base[0], 0, sizeof(GDT_base[0]));       // нулевой сегмент. всегда 0
    // SEG_SEL_CODE32 
    SetSegDesc(&GDT_base[1], 0xFFFFFFFF, 0x00000000,    // 32х битный сегмент кода
             SEGDESC_TYPE_EXECREAD_CODE,                // уровень привилегий 0
             SEGDESC_S_CODE_OR_DATA_SEGMENT, 0, 1,      // база 0 лимит 4G
             0, SEGDESC_L_16_OR_32, SEGDESC_D_B_32);
    // SEG_SEL_DATA32
    SetSegDesc(&GDT_base[2], 0xFFFFFFFF, 0x00000000,    // 32х битный сегмент данных
             SEGDESC_TYPE_RDWR_DATA,                    // уровень привилегий 0
             SEGDESC_S_CODE_OR_DATA_SEGMENT, 0, 1,      // база 0 лимит 4G
             0, SEGDESC_L_16_OR_32, SEGDESC_D_B_32);
    // SEG_SEL_CODE16
    SetSegDesc(&GDT_base[3], 0x0000FFFF, 0x00000000,    // 16ти битный сегмент кода
             SEGDESC_TYPE_EXECREAD_CODE,                // уровень привилегий 0
             SEGDESC_S_CODE_OR_DATA_SEGMENT, 0, 1,      // база 0 лимит 4G
             0, SEGDESC_L_16_OR_32, SEGDESC_D_B_16);
    // SEG_SEL_DATA16
    SetSegDesc(&GDT_base[4], 0x0000FFFF, 0x00000000,    // 16ти битный сегмент данных
             SEGDESC_TYPE_RDWR_DATA,                    // уровень привилегий 0
             SEGDESC_S_CODE_OR_DATA_SEGMENT, 0, 1,      // база 0 лимит 4G
             0, SEGDESC_L_16_OR_32, SEGDESC_D_B_16);

    struct descreg gdtr;

    gdtr.base = (ulong)GDT_base;            // указатель на сформированную таблицу
    gdtr.limit = 5 * sizeof(*GDT_base) - 1; // размер таблицы в байтах - 1
    __asm__ volatile ("lgdt %0"             // GCC-Inline-Assembly
                      :
                      : "m" (gdtr));
}