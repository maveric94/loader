#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "stdlib.h"

uint8 irq_base;
uint8 irq_count;

#define IRQ_HANDLER(name) void name(); \
	asm(#name ": pusha \n call _" #name " \n movb $0x20, %al \n outb %al, $0x20 \n outb %al, $0xA0 \n popa \n iret"); \
	void _ ## name()

void init_interrupts();
void set_int_handler(uint8 index, void *handler, uint8 type);
void timer_int_handler();
void keyboard_int_handler();

typedef struct {
	uint16 address_0_15;
	uint16 selector;
	uint8 reserved;
	uint8 type;
	uint16 address_16_31;
} __attribute__((packed)) IntDesc;

typedef struct {
	uint16 limit;
	void *base;
} __attribute__((packed)) IDTR;

IntDesc *idt = (void*)0xFFFFC000;

void init_interrupts() {
	*((size_t*)0xFFFFEFF0) = 0x8000 | 3;
	memset(idt, 0, 256 * sizeof(IntDesc));
	IDTR idtr = {256 * sizeof(IntDesc), idt};
	asm("lidt (,%0,)"::"a"(&idtr));
	irq_base = 0x20;
	irq_count = 16;
	outportb(0x20, 0x11);
	outportb(0x21, irq_base);
	outportb(0x21, 4);
	outportb(0x21, 1);
	outportb(0xA0, 0x11);
	outportb(0xA1, irq_base + 8);
	outportb(0xA1, 2);
	outportb(0xA1, 1);
	set_int_handler(irq_base, timer_int_handler, 0x8E);
	set_int_handler(irq_base + 1, keyboard_int_handler, 0x8E);
	asm("sti");
}

void set_int_handler(uint8 index, void *handler, uint8 type) {
	asm("pushf \n cli");
	idt[index].selector = 8;
	idt[index].address_0_15 = (size_t)handler & 0xFFFF;
	idt[index].address_16_31 = (size_t)handler >> 16;
	idt[index].type = type;
	idt[index].reserved = 0;
	asm("popf"); 
}

IRQ_HANDLER(timer_int_handler) {
	(*((char*)(0xB8000 + 79 * 2)))++;
}

IRQ_HANDLER(keyboard_int_handler) {
	uint8 key_code;
	inportb(0x60, key_code);
	printf("You pressed key with code %d\n", key_code);
	uint8 status;
	inportb(0x61, status);
	status |= 1;
	outportb(0x61, status);
} 

#endif
