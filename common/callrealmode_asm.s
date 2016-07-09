
SEG_SEL_CODE_REAL = 0x0000
SEG_SEL_DATA_REAL = 0x0000
CALLREALMODE_OFFSET = 0x5000
# это объявления из enum callrealmode_Func
CALLREALMODE_FUNC_GETSYSMEMMAP = 0x0
CALLREALMODE_FUNC_GETSCANCODE = 0x1
CALLREALMODE_FUNC_MOVECURSOR = 0x2
# эти объявления должны совпадать с тем что написано в segment.h
SEG_SEL_CODE32 = (1 * 8)
SEG_SEL_DATA32 = (2 * 8)
SEG_SEL_CODE16 = (3 * 8)
SEG_SEL_DATA16 = (4 * 8)
# объявления для использования меток в коде на С
.globl callrealmode_start
.globl callrealmode_end
# все что ниже - попадает в секцию text
.text
# после копирования на адрес CALLREALMODE_OFFSET,
# адрес callrealmode_start будет равен CALLREALMODE_OFFSET.
# таким образом из кода на С управление будет передаваться
# на метку callrealmode_start
.code16 # код ниже 16ти битный
callrealmode_start:

	push %ebp # формируем stackframe
	mov %sp,%bp
	call protection_off # после вызова этой функции код выполняется в RM
	call callrealmode_switch # код, содержащий обращение к BIOS
	call protection_on # включение PM
	mov %bp,%sp
	pop %ebp # удаляем stackframe

lretl # Возвращаемся в 32х битный код.
# на вершине стека в данный момент лежит
# селектор 32х битного дескриптора кода (CS) и
# регистр EIP
callrealmode_switch:

	OFF_FUNC = 44 # на таком смещении относительно %bp
	# находится поле func структуры callrealmode_Data
	# Which function?
	movw OFF_FUNC(%bp),%ax
	cmp $CALLREALMODE_FUNC_GETSYSMEMMAP,%ax
	je getsysmemmap
	cmp $CALLREALMODE_FUNC_GETSCANCODE, %ax
	je getscancode
	cmp $CALLREALMODE_FUNC_MOVECURSOR, %ax
	je movecursor

	ret



movecursor:
	
	OFF_MOVECURSOR_ROW = 50
	OFF_MOVECURSOR_COLUMN = 51
	OFF_MOVECURSOR_PAGE = 52

	push %bp
	mov %sp, %bp

	mov $0x02, %ah
	mov %ss:OFF_MOVECURSOR_PAGE(%bp), %bh
	mov %ss:OFF_MOVECURSOR_ROW(%bp), %dh
	mov %ss:OFF_MOVECURSOR_COLUMN(%bp), %dl

	int $0x10

	pop %bp

	ret

getscancode:

	OFF_GETSCANCODE_CODE = 50

	push %bp
	mov %sp, %bp

	#mov %ss,%ax
	#mov %ax,%es

	mov $0x00, %ah 
	int $0x16

	mov %ah, %ss:OFF_GETSCANCODE_CODE(%bp)

	pop %bp

	ret

getsysmemmap:

	OFF_GETSYSMEMMAP_EBX = 50 # смещение поля num в структуре callrealmode_Data
	OFF_GETSYSMEMMAP_EBX_RET = 54 # смещение поля next_num в структуре callrealmode_Data
	OFF_GETSYSMEMMAP_E820DATA = 58 # смещение поля base в структуре callrealmode_Data
	push %bp
	mov %sp,%bp
	# Call int $0x15
	stc # устанавливаем Carry флаг в EFLAGS
	# устанавливаем es:di = &param.getsysmemmap.base
	# по этому адресу будет сохраняться информация
	# о диапазоне физческой памяти
	mov %ss,%ax
	mov %ax,%es
	lea OFF_GETSYSMEMMAP_E820DATA(%bp),%di
	mov $0xE820,%eax
	mov $0x534D4150,%edx
	mov %ss:OFF_GETSYSMEMMAP_EBX(%bp),%ebx # устанавливаем EBX = param.getsysmemmap.num
	mov $20,%ecx # 20 = sizeof(param.getsysmemmap.base) +
	# sizeof(param.getsysmemmap.len) +
	# sizeof(param.getsysmemmap.type)
	int $0x15 # обращаемся к BIOS
	# Check error
	mov $1,%ax
	jc 1f # в случае ошибки BIOS выставит Carry флаг и jc перейдет по метке 1
	xor %ax,%ax
	1:
	# Save next %ebx
	mov %ebx,%ss:OFF_GETSYSMEMMAP_EBX_RET(%bp) # param.getsysmemmap.next_num = EBX
	pop %bp

	ret

protection_on:

	# выключаем прерывания
	cli
	# востанавливаем регистры gdtr и idtr
	lgdt saved_gdtr - callrealmode_start + CALLREALMODE_OFFSET
	lidt saved_idtr - callrealmode_start + CALLREALMODE_OFFSET
	# включаем PM
	mov %cr0,%ebx
	orl $0x1,%ebx
	mov %ebx,%cr0
	# загружаем в CS селектор 16ти битного сегмента кода и прыгаем на метку prot_mode
	ljmp $SEG_SEL_CODE16,$prot_mode - callrealmode_start + CALLREALMODE_OFFSET
	prot_mode: .code16
	# загружаем в регистры SS, DS, ES, GS, FS селектор 16ти битного сегмента данных
	mov $SEG_SEL_DATA16,%bx
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %ss
	mov %bx, %fs
	mov %bx, %gs

	ret

.code16

protection_off:
# сохраняем gdtr и idtr по адресам saved_gdtr и saved_idtr соответственно
# в нашем случае idtr можно было не сохранять
# эти значения понадобятся для возврата в PM
	sgdt saved_gdtr - callrealmode_start + CALLREALMODE_OFFSET
	sidt saved_idtr - callrealmode_start + CALLREALMODE_OFFSET
	# переходим в RM
	mov %cr0,%ebx
	andl $0xFFFFFFFE,%ebx
	mov %ebx,%cr0
	# загружаем в регистр CS базу кода SEG_SEL_CODE_REAL = 0 и прыгаем на метку real_mode
	ljmp $SEG_SEL_CODE_REAL,$real_mode - callrealmode_start + CALLREALMODE_OFFSET
	real_mode:
	# загружаем в регистры SS, DS, ES, GS, FS базу SEG_SEL_DATA_REAL = 0
	mov $SEG_SEL_DATA_REAL,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	mov %ax,%ss
	# загружаем в idtr указатель на таблицу прерываний BIOS и включаем прерывания.
	# это нужно что бы мы сами могли вызвать прерывание и что бы BIOS мог общаться с оборудованием.
	lidt real_mode_idtr - callrealmode_start + CALLREALMODE_OFFSET
	sti

	ret

real_mode_idtr:
.word 0x3ff,0,0
saved_gdtr: # по этому адресу сохраняется gdtr из PM
.word 0x0
.long 0x0
saved_idtr: # по этому адресу сохраняется idtr из PM
.word 0x0
.long 0x0

callrealmode_end: