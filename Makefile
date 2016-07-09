CC      = gcc
CFLAGS  = -Wall -fno-builtin -nostdinc -nostdlib -w -m32
LD      = ld

OBJFILES = \
	loader.o \
	kernel.o \
	common/string.o  \
	common/printf.o \
	common/screen.o \
    common/callrealmode.o \
    common/callrealmode_asm.o \
    common/bios.o  \
    common/vbe.o  \
    common/qdivrem.o  \
    common/udivdi3.o  \
    common/umoddi3.o  \
    common/divdi3.o  \
    common/moddi3.o  \
    common/setjmp.o  \
    common/s_floor.o  \
    x86emu/x86emu.o  \
    x86emu/x86emu_util.o  \
    fractal.o \
    common/descriptor.o   \
    common/pci_dev.o    \
    common/discs.o

image:
	@echo "Creating hdd.img..."
	@dd if=/dev/zero of=./hdd.img bs=512 count=16065 1>/dev/null 2>&1

	@echo "Creating bootable first FAT32 partition..."
	@losetup /dev/loop1 ./hdd.img
	@(echo c; echo u; echo n; echo p; echo 1; echo ;  echo ; echo a; echo 1; echo t; echo c; echo w;) | fdisk /dev/loop1 1>/dev/null 2>&1 || true

	@echo "Mounting partition to /dev/loop2..."
	@losetup /dev/loop2 ./hdd.img \
	--offset    `echo \`fdisk -lu /dev/loop1 | sed -n 10p | awk '{print $$3}'\`*512 | bc` \
	--sizelimit `echo \`fdisk -lu /dev/loop1 | sed -n 10p | awk '{print $$4}'\`*512 | bc`
	@losetup -d /dev/loop1

	@echo "Format partition..."
	@mkdosfs /dev/loop2

	@echo "Copy kernel and grub files on partition..."
	@mkdir -p tempdir
	@mount /dev/loop2 tempdir
	@mkdir tempdir/boot
	@cp -r grub tempdir/boot/
	@cp kernel.bin tempdir/
	@sleep 1
	@umount /dev/loop2
	@rm -r tempdir
	@losetup -d /dev/loop2

	@echo "Installing GRUB..."
	@echo "device (hd0) hdd.img \n \
           root (hd0,0)         \n \
           setup (hd0)          \n \
           quit\n" | grub --batch 1>/dev/null
	@echo "Done!"

all: kernel.bin
rebuild: clean all
.s.o:
	as --32 -Iinclude -o $@ $<
.c.o:
	$(CC) -Ix86emu -Iinclude $(CFLAGS) -o $@ -c $<
kernel.bin: $(OBJFILES)
	$(LD) -T linker.ld -o $@ $^
clean:
	rm -f $(OBJFILES) hdd.img kernel.bin
